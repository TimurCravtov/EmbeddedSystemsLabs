param(
    [string]$Folder = "reports/l3_1/Images/work",
    [string]$Format = "png"
)

$folderPath = Join-Path (Get-Location) $Folder

if (-not (Test-Path $folderPath)) {
    Write-Error "Folder $folderPath not found"
    exit 1
}

$files = Get-ChildItem -Path $folderPath -Filter *.drawio -Recurse

if ($files.Count -eq 0) {
    Write-Output "No .drawio files found in $folderPath"
    exit 0
}

$exeCandidates = @(
    'draw.io',
    'drawio',
    'C:\Program Files\draw.io\draw.io.exe',
    'C:\Program Files (x86)\draw.io\draw.io.exe',
    'C:\Program Files\diagrams.net\draw.io.exe',
    'C:\Program Files (x86)\diagrams.net\draw.io.exe'
)

$exePath = $null
foreach ($c in $exeCandidates) {
    try {
        $cmd = Get-Command $c -ErrorAction Stop
        $exePath = $cmd.Path
        break
    } catch {
    }
}

if (-not $exePath) {
    Write-Warning "draw.io executable not found. Install draw.io Desktop or ensure 'draw.io' is on PATH."
    $resp = Read-Host "draw.io not found. Attempt online export? (Y/N)"
    if ($resp -ne 'Y') { Write-Output "Aborted."; exit 1 }
    foreach ($f in $files) {
        $xml = Get-Content $f.FullName -Raw
        $body = @{ "format"=$Format; "xml"=$xml }
        try {
            $response = Invoke-RestMethod -Uri "https://exp.draw.io/ImageExport4" -Method Post -Body $body -TimeoutSec 120 -ErrorAction Stop
            $bytes = [System.Convert]::FromBase64String($response)
            $out = "$($f.DirectoryName)\$($f.BaseName).$Format"
            [IO.File]::WriteAllBytes($out, $bytes)
            Write-Output "Exported $out"
        } catch {
            Write-Warning "Failed exporting $($f.FullName): $_"
        }
    }
    exit 0
}

# Use local draw.io
foreach ($f in $files) {
    $out = "$($f.DirectoryName)\$($f.BaseName).$Format"
    Write-Output "Exporting $($f.Name) -> $out using $exePath"
    & $exePath --export --format $Format --output $out $f.FullName
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Export failed for $($f.FullName)"
    } else {
        Write-Output "Exported: $out"
    }
}
