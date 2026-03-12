param(
    [string]$ImageTag = "sandboxrunner-jammy-release",
    [string]$ArtifactsDir = "out/artifacts/linux-jammy-release"
)

$ErrorActionPreference = "Stop"

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptRoot
$dockerfile = Join-Path $projectRoot "docker/jammy-release.Dockerfile"
$outputRoot = Join-Path $projectRoot $ArtifactsDir

New-Item -ItemType Directory -Force -Path $outputRoot | Out-Null

& docker build `
    --file $dockerfile `
    --tag $ImageTag `
    $projectRoot
if ($LASTEXITCODE -ne 0) {
    throw "Docker build failed."
}

$containerId = (& docker create $ImageTag).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($containerId)) {
    throw "Docker create failed."
}

try {
    & docker cp "${containerId}:/artifacts/." $outputRoot
    if ($LASTEXITCODE -ne 0) {
        throw "Docker copy failed."
    }
}
finally {
    if (-not [string]::IsNullOrWhiteSpace($containerId)) {
        & docker rm -f $containerId | Out-Null
    }
}

Write-Host "Exported release artifacts to $outputRoot"
