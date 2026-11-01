# ENGINEX Workbench - Standalone Turnkey Packaging Script
# PASS Science Works R&D Internship 2026-2027 Evaluation

$ErrorActionPreference = 'Stop'

Write-Host '================================================================='
Write-Host '  ENGINEX Workbench - Packaging Release ZIP'
Write-Host '================================================================='

$ProjectRoot = (Get-Item $PSScriptRoot).Parent.Parent.FullName
Set-Location $ProjectRoot

$DistDir = Join-Path $ProjectRoot 'dist'
$BundleDir = Join-Path $DistDir 'CandidateID_Workbench'
$ZipFile = Join-Path $DistDir 'CandidateID_Workbench.zip'

if (Test-Path $DistDir) {
    Write-Host '[INFO] Cleaning previous distribution folder...'
    Remove-Item -Recurse -Force $DistDir
}
New-Item -ItemType Directory -Force -Path $BundleDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $BundleDir 'bin') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $BundleDir 'data\geometry') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $BundleDir 'data\temperature') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $BundleDir 'results\csv') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $BundleDir 'results\benchmarks') | Out-Null

Write-Host '[INFO] Copying Launch.exe...'
Copy-Item (Join-Path $ProjectRoot 'build\bin\Launch.exe') (Join-Path $BundleDir 'Launch.exe')

Write-Host '[INFO] Copying backend_server.exe...'
Copy-Item (Join-Path $ProjectRoot 'build\bin\backend_server.exe') (Join-Path $BundleDir 'bin\backend_server.exe')

Write-Host '[INFO] Copying static web export to web/...'
Copy-Item -Recurse (Join-Path $ProjectRoot 'frontend\out') (Join-Path $BundleDir 'web')

Write-Host '[INFO] Copying sample fixtures...'
Copy-Item (Join-Path $ProjectRoot 'data\geometry\sample_cube.stl') (Join-Path $BundleDir 'data\geometry\sample_cube.stl')
Copy-Item (Join-Path $ProjectRoot 'data\temperature\synthetic_ansys_sample.csv') (Join-Path $BundleDir 'data\temperature\synthetic_ansys_sample.csv')

Write-Host '[INFO] Copying reference CSV outputs...'
if (Test-Path (Join-Path $ProjectRoot 'results\csv\thermal_convergence.csv')) {
    Copy-Item (Join-Path $ProjectRoot 'results\csv\thermal_convergence.csv') (Join-Path $BundleDir 'results\csv\thermal_convergence.csv')
}
if (Test-Path (Join-Path $ProjectRoot 'results\benchmarks\benchmark_results.csv')) {
    Copy-Item (Join-Path $ProjectRoot 'results\benchmarks\benchmark_results.csv') (Join-Path $BundleDir 'results\benchmarks\benchmark_results.csv')
}
if (Test-Path (Join-Path $ProjectRoot 'results\csv\backpropagation_training.csv')) {
    Copy-Item (Join-Path $ProjectRoot 'results\csv\backpropagation_training.csv') (Join-Path $BundleDir 'results\csv\backpropagation_training.csv')
}

Write-Host '[INFO] Copying README.txt...'
Copy-Item (Join-Path $PSScriptRoot 'README_DIST.txt') (Join-Path $BundleDir 'README.txt')

Write-Host ('[INFO] Creating release archive: ' + $ZipFile + '...')
Compress-Archive -Path ($BundleDir + '\*') -DestinationPath $ZipFile -Force

$ZipSizeMB = [math]::Round((Get-Item $ZipFile).Length / 1MB, 2)
Write-Host '================================================================='
Write-Host ('  SUCCESS! Release package generated: ' + $ZipFile)
Write-Host ('  Package Size: ' + $ZipSizeMB + ' MB')
Write-Host '================================================================='
