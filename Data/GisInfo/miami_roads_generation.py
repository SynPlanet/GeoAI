import osmnx as ox
import requests
import json
import numpy as np
from shapely.geometry import LineString, Point, Polygon, MultiLineString
from shapely.ops import transform, nearest_points
from shapely.strtree import STRtree
from pyproj import Geod, Transformer
import logging
import time
import pickle
import os
from tqdm import tqdm

# --- Settings ---
CITY_CENTER = (25.7743, -80.1937)
DIST_METERS = 10000
OUTPUT_GEOJSON = "miami_roads_3d.geojson"
CACHE_FILE = "elev_cache.pkl"
API_URL = "https://api.opentopodata.org/v1/srtm90m"
MAX_ELEV = 100
INITIAL_BATCH_SIZE = 100
geod = Geod(ellps="WGS84")

logging.basicConfig(level=logging.INFO, format="%(message)s")
logger = logging.getLogger("3D-Road")


# --- Elevation Cache ---
def round_coord(lat, lon):
    return (round(lat, 7), round(lon, 7))

def load_cache(path):
    if os.path.exists(path):
        with open(path, "rb") as f:
            return pickle.load(f)
    return {}

def save_cache(cache, path):
    with open(path, "wb") as f:
        pickle.dump(cache, f)

def query_elevation_batch(latlons):
    batch_size = len(latlons)
    while batch_size > 0:
        try:
            locations = "|".join(f"{lat},{lon}" for lat, lon in latlons[:batch_size])
            resp = requests.get(API_URL, params={"locations": locations})
            if resp.status_code == 200:
                return [(r["location"]["lat"], r["location"]["lng"], r["elevation"])
                        for r in resp.json()["results"] if "elevation" in r]
            elif resp.status_code in [414, 429]:
                batch_size //= 2
                time.sleep(0.5)
            else:
                logger.warning(f"❌ API Error {resp.status_code}: {resp.text[:100]}")
                break
        except Exception as e:
            logger.warning(f"⚠️ Request Error: {e}")
            break
    return []

# --- Geometry ---
def densify_linestring(geom, spacing=10):
    if geom.length < spacing:
        return geom
    num_points = int(geom.length // spacing) + 2
    points = [geom.interpolate(i / (num_points - 1), normalized=True) for i in range(num_points)]
    return LineString(points)

def parse_lanes(value):
    if isinstance(value, list):
        value = value[0]
    if isinstance(value, str):
        parts = [p.strip() for p in value.replace(';', ',').split(',')]
        nums = [int(p) for p in parts if p.isdigit()]
        return max(nums) if nums else 1
    if isinstance(value, (int, float)):
        return int(value)
    return 1

def parse_maxspeed(value):
    if isinstance(value, list):
        value = value[0]
    if isinstance(value, str):
        parts = [p.strip() for p in value.replace(';', ',').split(',')]
        nums = [int(p) for p in parts if p.isdigit()]
        return max(nums) if nums else "unknown"
    if isinstance(value, (int, float)):
        return int(value)
    return "unknown"

def get_intersections(nodes, min_street_count=2):
    return set(nodes[nodes["street_count"] >= min_street_count].index)

from shapely.ops import unary_union

def merge_close_intersections(polygons, threshold_deg=0.00015):
    merged = []
    used = set()
    for i, p1 in enumerate(polygons):
        if i in used:
            continue
        cluster = [p1]
        used.add(i)
        for j, p2 in enumerate(polygons):
            if j == i or j in used:
                continue
            if p1.distance(p2) < threshold_deg:  # 👈 instead of .centroid
                cluster.append(p2)
                used.add(j)
        merged.append(unary_union(cluster).buffer(0))
    return merged

# --- Rotation and Intersection Functions ---
def rotate_point(x, y, angle_deg):
    angle_rad = math.radians(angle_deg)
    x_rot = x * math.cos(angle_rad) - y * math.sin(angle_rad)
    y_rot = x * math.sin(angle_rad) + y * math.cos(angle_rad)
    return x_rot, y_rot

from shapely.geometry import Point
from geopy.distance import geodesic

# Function to remove duplicate points and points that are too close to each other
def filter_unique_points(coords, min_distance=1.0):
    unique_coords = []
    seen_coords = set()

    for i, (lon, lat) in enumerate(coords):
        point = (lon, lat)
        
        # Check if the point has already been added (considering the minimum distance)
        if point in seen_coords:
            continue
        
        # Check the distance to the already added points
        if len(unique_coords) > 0:
            last_point = unique_coords[-1]
            distance = geodesic(last_point, point).meters
            if distance < min_distance:
                continue  # Skip the point if it is too close to the previous one
        
        # Add the unique point
        unique_coords.append((lon, lat))
        seen_coords.add(point)

    return unique_coords

# --- Intersections ---
def build_intersection_polygon(node_id, G, base_radius=8.0, rotation_angle=45.0):
    try:
        lon, lat = G.nodes[node_id]["x"], G.nodes[node_id]["y"]
        to_meters = Transformer.from_crs("epsg:4326", "epsg:3857", always_xy=True).transform
        to_latlon = Transformer.from_crs("epsg:3857", "epsg:4326", always_xy=True).transform
        cx, cy = to_meters(lon, lat)

        half = base_radius
        square_coords = [
            (cx - half, cy - half),
            (cx + half, cy - half),
            (cx + half, cy + half),
            (cx - half, cy + half),
            (cx - half, cy - half)
        ]

        # Rotate the square by 45 degrees
        rotated_square_coords = [rotate_point(x - cx, y - cy, rotation_angle) for x, y in square_coords]
        rotated_square_coords = [(x + cx, y + cy) for x, y in rotated_square_coords]

        rotated_polygon = Polygon(rotated_square_coords)

        return transform(to_latlon, rotated_polygon).buffer(0)

    except Exception as e:
        logger.warning(f"Error at intersection {node_id}: {e}")
        return None

import math

def angle_between_vectors(a, b):
    dot = a[0] * b[0] + a[1] * b[1]
    mag_a = (a[0]**2 + a[1]**2)**0.5
    mag_b = (b[0]**2 + b[1]**2)**0.5
    if mag_a == 0 or mag_b == 0:
        return 0
    cos_angle = max(min(dot / (mag_a * mag_b), 1), -1)
    angle_rad = math.acos(cos_angle)
    angle_deg = math.degrees(angle_rad)
    return angle_deg

def get_direction_vector(center_node, edge):
    u, v, data = edge
    if 'geometry' in data and isinstance(data['geometry'], LineString):
        coords = list(data['geometry'].coords)
        if len(coords) < 2:
            return None

        if u == center_node:
            x0, y0 = coords[0]
            x1, y1 = coords[1]
        elif v == center_node:
            x0, y0 = coords[-1]
            x1, y1 = coords[-2]
        else:
            return None

        dx = x1 - x0
        dy = y1 - y0
        length = (dx**2 + dy**2)**0.5
        if length == 0:
            return None
        return (dx / length, dy / length)
    return None

def is_real_intersection(node_id, G, min_angle_deg=20):
    edges = list(G.edges(node_id, data=True))
    if len(edges) < 2:
        return False

    # Processing T-junctions (exactly 2 segments)
    if len(edges) == 2:
        a = get_direction_vector(node_id, edges[0])
        b = get_direction_vector(node_id, edges[1])
        if a and b:
            angle = angle_between_vectors(a, b)
            if angle >= min_angle_deg:
                logger.debug(f"✅ T-node {node_id} is a real intersection. Angle: {angle}")
                return True
            else:
                logger.debug(f"❌ T-node {node_id} rejected. Angle: {angle}")
                return False

    # Standard logic for >=3 segments
    angles = []
    for i in range(len(edges)):
        for j in range(i+1, len(edges)):
            a = get_direction_vector(node_id, edges[i])
            b = get_direction_vector(node_id, edges[j])
            if a and b:
                angle = angle_between_vectors(a, b)
                angles.append(angle)

    if any(angle >= min_angle_deg for angle in angles):
        logger.debug(f"✅ Node {node_id} is a real intersection. Angles: {angles}")
        return True
    else:
        logger.debug(f"❌ Node {node_id} rejected. Angles: {angles}")
        return False


    angles = []
    for i in range(len(edges)):
        for j in range(i+1, len(edges)):
            a = get_direction_vector(node_id, edges[i])
            b = get_direction_vector(node_id, edges[j])
            if a is not None and b is not None:
                angle = angle_between_vectors(a, b)
                angles.append(angle)

    return any(a > min_angle_deg for a in angles)


# --- Start ---
logger.info("🌐 Loading road network from OpenStreetMap.")
G = ox.graph_from_point(CITY_CENTER, dist=DIST_METERS, network_type="drive", simplify=False)
G = ox.simplify_graph(G)
nodes, edges = ox.graph_to_gdfs(G)

logger.info("📡 Loading elevation data.")
elevation_cache = load_cache(CACHE_FILE)
all_coords = set()
for geom in edges.geometry:
    if geom and not geom.is_empty:
        for lon, lat in geom.coords:
            rounded = round_coord(lat, lon)
            if rounded not in elevation_cache:
                all_coords.add((lat, lon))

coords_list = list(all_coords)
for i in tqdm(range(0, len(coords_list), INITIAL_BATCH_SIZE)):
    sub_batch = coords_list[i:i + INITIAL_BATCH_SIZE]
    result = query_elevation_batch(sub_batch)
    for lat, lon, z in result:
        elevation_cache[round_coord(lat, lon)] = z
save_cache(elevation_cache, CACHE_FILE)

sample_coords = [pt for geom in edges.geometry if geom and not geom.is_empty for pt in geom.coords][:3000]
elevations = [elevation_cache.get(round_coord(lat, lon)) for lon, lat in sample_coords]
elevations = [e for e in elevations if e and 0 < e < MAX_ELEV]
city_median = int(np.median(elevations))
logger.info(f"📏 City median elevation: {city_median} m")

roundabout_nodes = set()
for u, v, k, data in G.edges(keys=True, data=True):
    if data.get("junction") == "roundabout":
        roundabout_nodes.update([u, v])

# --- Intersection Identification ---
all_nodes = set(G.nodes)
intersection_ids = set()

# 1. Main intersections: not part of a roundabout + geometric filtering
for node_id in all_nodes:
    if node_id not in roundabout_nodes and is_real_intersection(node_id, G):
        intersection_ids.add(node_id)

# 2. Additional: neighboring nodes of roundabouts (entrances/exits)
for round_node in roundabout_nodes:
    for neighbor in G.neighbors(round_node):
        if neighbor not in roundabout_nodes and is_real_intersection(neighbor, G):
            intersection_ids.add(neighbor)

raw_polygons = []
logger.info("🏗️ Building raw intersection polygons.")
for node_id in tqdm(intersection_ids):
    polygon = build_intersection_polygon(node_id, G)
    if polygon and polygon.is_valid and isinstance(polygon, Polygon):
        raw_polygons.append(polygon)

logger.info(f"🔀 Merging close intersections (up to {len(raw_polygons)})...")
poly_shapes = merge_close_intersections(raw_polygons)

intersection_polygons = []
for i, polygon in enumerate(poly_shapes):
    if polygon.geom_type == "Polygon":
        coords_list = [list(polygon.exterior.coords)]
    elif polygon.geom_type == "MultiPolygon":
        coords_list = [list(p.exterior.coords) for p in polygon.geoms]
    else:
        logger.warning(f"⚠️ Unknown geometry type of intersection: {polygon.geom_type}")
        continue

    intersection_polygons.append({
        "type": "Feature",
        "geometry": {
            "type": "MultiPolygon" if polygon.geom_type == "MultiPolygon" else "Polygon",
            "coordinates": [coords_list] if polygon.geom_type == "MultiPolygon" else coords_list
        },
        "properties": {
            "id": f"intersection_{i}"
        }
    })

from shapely.geometry import Polygon, LineString

from shapely.geometry import Polygon, MultiPolygon, LineString

def get_midpoint_on_edge(polygon):
    """Gets the midpoint of each side of the polygon"""
    coords = list(polygon.exterior.coords)
    midpoints = []
    
    for i in range(len(coords) - 1):
        p1 = coords[i]
        p2 = coords[i + 1]
        
        # Find the midpoint of the side
        midpoint = ((p1[0] + p2[0]) / 2, (p1[1] + p2[1]) / 2)
        midpoints.append(midpoint)
        
    return midpoints

def check_segments_between_intersections(intersection_polygons, features):
    """
    Checks if there are segments between intersections after processing.
    If segments are missing, it tries to recover them, if possible.
    """
    for i in range(1, len(intersection_polygons)):
        prev_polygon_data = intersection_polygons[i-1]["geometry"]
        curr_polygon_data = intersection_polygons[i]["geometry"]
        
        # Check what data we have
        print(f"prev_polygon_data: {prev_polygon_data}")
        print(f"curr_polygon_data: {curr_polygon_data}")

        # Convert coordinates to the correct format (lists of numbers)
        prev_polygon_coords = prev_polygon_data["coordinates"][0] if isinstance(prev_polygon_data, dict) else prev_polygon_data
        curr_polygon_coords = curr_polygon_data["coordinates"][0] if isinstance(curr_polygon_data, dict) else curr_polygon_data

        # Check that the obtained coordinates are lists of tuples
        print(f"prev_polygon_coords: {prev_polygon_coords}")
        print(f"curr_polygon_coords: {curr_polygon_coords}")

        # Check geometry type: if MultiPolygon, process each polygon
        if isinstance(prev_polygon_coords[0], list):  # MultiPolygon
            prev_polygon_coords = [list(map(float, coord)) for sublist in prev_polygon_coords for coord in sublist]
        else:  # Polygon
            prev_polygon_coords = [list(map(float, coord)) for coord in prev_polygon_coords]
        
        if isinstance(curr_polygon_coords[0], list):  # MultiPolygon
            curr_polygon_coords = [list(map(float, coord)) for sublist in curr_polygon_coords for coord in sublist]
        else:  # Polygon
            curr_polygon_coords = [list(map(float, coord)) for coord in curr_polygon_coords]

        # Create polygons
        prev_polygon = Polygon(prev_polygon_coords) if isinstance(prev_polygon_coords[0], list) else Polygon([prev_polygon_coords])
        curr_polygon = Polygon(curr_polygon_coords) if isinstance(curr_polygon_coords[0], list) else Polygon([curr_polygon_coords])

        # Get midpoints on the boundary of the intersections
        prev_midpoints = get_midpoint_on_edge(prev_polygon)
        curr_midpoints = get_midpoint_on_edge(curr_polygon)
        
        # Create a segment between the nearest midpoints
        for start_point in prev_midpoints:
            for end_point in curr_midpoints:
                new_segment = LineString([start_point, end_point])
                logger.info(f"Created segment: {new_segment}")
                
                features.append({
                    "type": "Feature",
                    "geometry": {"type": "LineString", "coordinates": list(new_segment.coords)},
                    "properties": {"id": f"recovered_segment_{i-1}_{i}"}
                })

# --- Function for cutting segments at intersections ---
def cut_segment_at_intersections(segment):
    logger.info(f"Processing segment: {segment}")
    segments_to_process = [segment]
    for polygon in poly_shapes:
        if not isinstance(polygon, Polygon):
            continue

        if not segment.intersects(polygon):
            continue

        polygon = polygon.buffer(1e-6)  # Small buffer for accuracy
        new_segments = []
        for seg in segments_to_process:
            logger.info(f"  Checking segment: {seg}")
            if not seg.intersects(polygon):
                new_segments.append(seg)
                continue

            # Cutting the segment based on the intersection with the intersection
            outside = seg.difference(polygon)

            if isinstance(outside, LineString):
                outside = [outside]
            elif isinstance(outside, MultiLineString):
                outside = list(outside.geoms)
            else:
                outside = []

            for part in outside:
                logger.info(f"    Cutting: {part}")
                if not part or part.is_empty or len(part.coords) < 1:
                    logger.debug("⚠️ Skipping empty geometry during intersection cutting.")
                    continue

                start = Point(part.coords[0])
                end = Point(part.coords[-1])

                if polygon.contains(part):
                    logger.warning(f"⚠️ Segment found inside the intersection: {part}")
                    continue

                if polygon.contains(start):
                    edge = nearest_points(polygon.exterior, start)[0]
                    part = LineString([edge.coords[0]] + list(part.coords[1:]))

                if polygon.contains(end):
                    edge = nearest_points(polygon.exterior, end)[0]
                    part = LineString(list(part.coords[:-1]) + [edge.coords[0]])

                logger.info(f"    Adding segment: {part}")
                if len(part.coords) >= 2:
                    new_segments.append(part)

        segments_to_process = new_segments

    logger.info(f"Final segments: {segments_to_process}")
    return segments_to_process


# --- Road Segment Generation ---
logger.info("🛣️ Processing road segments.")
features = []
segment_id = 0

for u, v, key, data in tqdm(G.edges(keys=True, data=True)):
    geom = data.get("geometry")
    if not geom or geom.is_empty:
        continue

    highway = data.get("highway")
    name = data.get("name") or ""
    lanes = parse_lanes(data.get("lanes", 1))
    width = float(data.get("width", lanes * 3.5))
    maxspeed = parse_maxspeed(data.get("maxspeed", "unknown"))
    oneway = data.get("oneway", False)
    access = data.get("access", "yes")
    hgv = data.get("hgv", "yes")
    bridge = data.get("bridge") == "yes"
    motorway_link = highway in ["motorway_link", "trunk_link"]
    junction_type = data.get("junction")
    is_roundabout = data.get("junction") == "roundabout"
    u_round = G.nodes[u].get("junction") == "roundabout"
    v_round = G.nodes[v].get("junction") == "roundabout"
    is_entry_exit = u_round != v_round

    segments = cut_segment_at_intersections(geom)

    for segment in segments:
        dense = densify_linestring(segment, spacing=10)
        coords3d = []
        for lon, lat in dense.coords:
            z = elevation_cache.get(round_coord(lat, lon))
            if z is None:
                continue
            coords3d.append((lon, lat, z))

        if len(coords3d) < 2:
            continue

        seen = set()
        filtered = []
        # Filter segments by length
        for lon, lat, z in coords3d:
            key = (round(lon, 7), round(lat, 7))
            if key not in seen:
                seen.add(key)
                filtered.append((lon, lat, z))

        if len(filtered) < 2:
            logger.warning(f"⚠️ Segment too short or empty: {filtered}")  # Log filtered segments
            continue

        avg_z = np.mean([z for _, _, z in filtered])
        if avg_z < (0.5 * city_median):
            continue

        features.append({
            "type": "Feature",
            "geometry": {"type": "LineString", "coordinates": filtered},
            "properties": {
                "id": f"segment_{segment_id}",
                "name": name,
                "highway": highway,
                "lanes": lanes,
                "width": width,
                "maxspeed": maxspeed,
                "direction": "oneway" if oneway else "both",
                "vehicle_types_allowed": access,
                "long_truck_access": hgv != "no",
                "is_motorway_entry": motorway_link,
                "is_motorway_exit": motorway_link,
                "bridge": bridge,
                "is_roundabout": is_roundabout,
                "is_roundabout_entry_exit": is_entry_exit
            }
        })
        segment_id += 1


# --- Checking segments between intersections ---
logger.info("🔍 Checking segments between intersections.")
#check_segments_between_intersections(intersection_polygons, features)


# --- Saving ---
logger.info("💾 Saving to GeoJSON...")
with open(OUTPUT_GEOJSON, "w", encoding="utf-8") as f:
    json.dump({"type": "FeatureCollection", "features": features + intersection_polygons}, f, indent=2)
logger.info(f"✅ Done. Segments: {segment_id}, Intersections: {len(intersection_polygons)}")
