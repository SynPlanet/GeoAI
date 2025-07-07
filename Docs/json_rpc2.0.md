## JSON-RPC 2.0: methods for interaction between UEapp and Frontend

### 📌 Table of methods from ueapp to frontend

| **Method name** | **Call parameters**                  | **Return value**            | **Description**                                                                                                                                                                |
| ----------------------------- | ---------------------------------------------------------- | -------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| sendTimerPolygonCoordinates   | `{ "coordinates": [lon,lat], "radius": meters }` | `{ "succsess": true }` or `{ "error": "Invalid data" }` | Sends the center of a circle and its radius in meters `[lon, lat]` (longitude and latitude in WGS84 format). |
| sendSearchAreaStartScale   | `{"scale": 0.0-1.0}` | `{}` | Sends data about the starting scale of the search zone to the frontend. |
| sendScreenTouchAsWGS   | `{ "coordinate": [X,Y,Z] }` | `{}` | Sends the frontend the touch point of the screen in the world converted to WGS, i.e. LLH. |
---


### 📌 Table of methods from frontend to ueapp

| **Method name** | **Call parameters**                  | **Return value**            | **Описание**                                                                                                                                                                |
| ----------------------------- | ---------------------------------------------------------- | -------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| onUserChanged   | `{}` | `{}` | Sends a message about user connection. |
| onChangeActiveStateSearchArea   | `{"active": true/false}` | `{}` | Sends an event about the need to switch the search zone state on/off. |
| onGetSearchAreaScale   | `{"scale": 0.0-1.0}` | `{}` | Notification about search area scaling change via frontend UI. |
| onBackToOrigin   | `{}` | `{}` | Notification of the need to return to the starting point. |
| onScanResponseReceived   | `{}` | `{}` | Notification about completion of the scanning process of objects in the zone. |
---
