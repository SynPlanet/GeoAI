$linuxPath = Join-Path $PSScriptRoot "Linux"
$scriptSource = Join-Path $linuxPath "GeoAi.sh"
$dockerfileSource = Join-Path $PSScriptRoot "Dockerfile"
$dockerfileDest = Join-Path $linuxPath "Dockerfile"

if (!(Test-Path $linuxPath)) {
    Write-Error "Linux folder doesn't existed"
    exit 1
}

if (Test-Path $scriptSource) {
    Copy-Item -Path $scriptSource -Destination $scriptCopy -Force
} else {
    Write-Error "File GeoAi.sh doesn't existed"
    exit 1
}

if (Test-Path $dockerfileSource) {
    Copy-Item -Path $dockerfileSource -Destination $dockerfileDest -Force
} else {
    Write-Error "Dockerfile doesn't existed in root folder"
    exit 1
}

$hashFull = Get-FileHash -Path $scriptCopy -Algorithm SHA256
$hash = $hashFull.Hash.Substring(0, 8).ToLower()

#$tag = "daily"
$tag = "test"
#$tag = "latest"
#$tag = "debug"
$image = "registry.georef.ai/georef.ai-game:$tag"

Set-Location $linuxPath

Write-Host "Build Docker-iso with tag: $tag"
$buildResult = docker build -t $image .
if ($LASTEXITCODE -ne 0) {
    Write-Error "Error on build Docker-iso"
    exit 1
}

# Public build
Write-Host "Publishing build in registry: $image"
$pushResult = docker push $image
if ($LASTEXITCODE -ne 0) {
    Write-Error "Docker-iso push error"
    exit 1
}

Write-Host "Publishing successed. Used tag: $tag"
