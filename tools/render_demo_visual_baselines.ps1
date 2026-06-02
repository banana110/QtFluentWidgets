param(
    [Parameter(Mandatory = $true)]
    [string] $Executable,

    [Parameter(Mandatory = $true)]
    [string] $OutputDir,

    [Parameter(Mandatory = $true)]
    [string] $QtBinDir,

    [string[]] $Pages = @("overview", "basic_input", "inputs", "buttons", "icons", "motion", "pickers", "angles", "dataviews", "containers", "windows", "settings"),
    [string[]] $Variants = @("light", "dark", "accent"),
    [string[]] $ScaleFactors = @("1", "1.5"),
    [string] $Platform = "windows"
)

$ErrorActionPreference = "Stop"

$exe = (Resolve-Path -LiteralPath $Executable).Path
$exeDir = Split-Path -Parent $exe

if (-not (Test-Path -LiteralPath $OutputDir)) {
    New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
}
$resolvedOutput = (Resolve-Path -LiteralPath $OutputDir).Path
Get-ChildItem -LiteralPath $resolvedOutput -Filter "*.png" -File -ErrorAction SilentlyContinue | Remove-Item -Force
Remove-Item -LiteralPath (Join-Path $resolvedOutput "_capture.log") -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath (Join-Path $resolvedOutput "_render.log") -Force -ErrorAction SilentlyContinue

$pageItems = ($Pages -join ",").Split(",", [System.StringSplitOptions]::RemoveEmptyEntries) | ForEach-Object { $_.Trim() } | Where-Object { $_ }
$variantItems = ($Variants -join ",").Split(",", [System.StringSplitOptions]::RemoveEmptyEntries) | ForEach-Object { $_.Trim() } | Where-Object { $_ }
$scaleItems = ($ScaleFactors -join ",").Split(",", [System.StringSplitOptions]::RemoveEmptyEntries) | ForEach-Object { $_.Trim() } | Where-Object { $_ }
$pageArg = $pageItems -join ","
$variantArg = $variantItems -join ","

function ConvertTo-SafeBaselineName {
    param([Parameter(Mandatory = $true)][string] $Text)

    $safe = $Text -replace "[^A-Za-z0-9_.-]+", "_"
    if ($safe.Length -gt 120) {
        return $safe.Substring(0, 120)
    }
    return $safe
}

function ConvertTo-RenderVariantOutputName {
    param([Parameter(Mandatory = $true)][string] $Name)

    $normalized = $Name.Trim().ToLowerInvariant()
    if ($normalized -eq "dark") {
        return "dark"
    }
    if ($normalized -eq "accent" -or $normalized -eq "light-accent") {
        return "light-accent"
    }
    if ($normalized -eq "dark-accent") {
        return "dark-accent"
    }
    return "light"
}

function Test-RenderedPngHealth {
    param(
        [Parameter(Mandatory = $true)]
        [string] $Directory,

        [Parameter(Mandatory = $true)]
        [string] $Filter,

        [Parameter(Mandatory = $true)]
        [int] $MinimumCount
    )

    $files = @(Get-ChildItem -LiteralPath $Directory -Filter $Filter -File -ErrorAction SilentlyContinue)
    if ($files.Count -lt $MinimumCount) {
        Write-Error "Expected at least $MinimumCount rendered images, captured $($files.Count)."
        return $false
    }

    try {
        Add-Type -AssemblyName System.Drawing -ErrorAction Stop
    } catch {
        Write-Warning "System.Drawing is unavailable; skipped PNG health validation."
        return $true
    }

    foreach ($file in $files) {
        if ($file.Length -lt 1024) {
            Write-Error "Rendered image $($file.Name) is unexpectedly small ($($file.Length) bytes)."
            return $false
        }

        $image = [System.Drawing.Bitmap]::FromFile($file.FullName)
        try {
            if ($image.Width -lt 320 -or $image.Height -lt 240) {
                Write-Error "Rendered image $($file.Name) has unexpected dimensions $($image.Width)x$($image.Height)."
                return $false
            }

            $sampleColors = @{}
            $stepX = [Math]::Max(1, [Math]::Floor($image.Width / 18))
            $stepY = [Math]::Max(1, [Math]::Floor($image.Height / 18))
            for ($y = [Math]::Floor($stepY / 2); $y -lt $image.Height; $y += $stepY) {
                for ($x = [Math]::Floor($stepX / 2); $x -lt $image.Width; $x += $stepX) {
                    $sampleColors[$image.GetPixel($x, $y).ToArgb()] = $true
                }
            }

            if ($sampleColors.Count -lt 3) {
                Write-Error "Rendered image $($file.Name) appears blank or nearly blank; sampled $($sampleColors.Count) distinct colors."
                return $false
            }
        } finally {
            $image.Dispose()
        }
    }

    return $true
}

function Test-RenderedMatrixCompleteness {
    param(
        [Parameter(Mandatory = $true)]
        [string] $Directory,

        [Parameter(Mandatory = $true)]
        [string[]] $Pages,

        [Parameter(Mandatory = $true)]
        [string[]] $VariantNames,

        [Parameter(Mandatory = $true)]
        [int] $ExpectedPerPageVariant
    )

    $files = @(Get-ChildItem -LiteralPath $Directory -Filter "*.png" -File -ErrorAction SilentlyContinue)
    foreach ($variant in $VariantNames) {
        foreach ($page in $Pages) {
            $safePage = ConvertTo-SafeBaselineName -Text $page
            $pattern = "$variant-scale*-$safePage.png"
            $matches = @($files | Where-Object { $_.Name -like $pattern })
            if ($matches.Count -ne $ExpectedPerPageVariant) {
                Write-Error "Expected $ExpectedPerPageVariant visual baselines for $variant/$page, captured $($matches.Count)."
                return $false
            }
        }
    }

    return $true
}

if ($pageItems.Count -eq 0) {
    Write-Error "No visual baseline pages were requested."
    exit 1
}
if ($variantItems.Count -eq 0) {
    Write-Error "No visual baseline variants were requested."
    exit 1
}
if ($scaleItems.Count -eq 0) {
    Write-Error "No visual baseline scale factors were requested."
    exit 1
}

$variantOutputNames = @($variantItems | ForEach-Object { ConvertTo-RenderVariantOutputName -Name $_ })
if (($variantOutputNames | Select-Object -Unique).Count -ne $variantOutputNames.Count) {
    Write-Error "Visual baseline variants map to duplicate output names: $($variantOutputNames -join ', ')."
    exit 1
}

$originalPath = $env:PATH
$originalPlatform = $env:QT_QPA_PLATFORM
$originalScaleFactor = $env:QT_SCALE_FACTOR
$originalScreenScaleFactors = $env:QT_SCREEN_SCALE_FACTORS
$originalSuppressWarnings = $env:QTFLUENT_SUPPRESS_KNOWN_PAINT_WARNINGS
$failureCode = 0

try {
    $env:PATH = "$exeDir;$QtBinDir;$env:PATH"
    $env:QTFLUENT_SUPPRESS_KNOWN_PAINT_WARNINGS = "1"
    if ($Platform) {
        $env:QT_QPA_PLATFORM = $Platform
    }
    if (Test-Path Env:QT_SCREEN_SCALE_FACTORS) {
        Remove-Item Env:QT_SCREEN_SCALE_FACTORS
    }

    foreach ($scale in $scaleItems) {
        $env:QT_SCALE_FACTOR = $scale
        Write-Host "Rendering visual baselines with QT_QPA_PLATFORM=$env:QT_QPA_PLATFORM QT_SCALE_FACTOR=$scale"
        $process = Start-Process `
            -FilePath $exe `
            -ArgumentList @("--render-baselines", $resolvedOutput, "--render-pages", $pageArg, "--render-variants", $variantArg) `
            -Wait `
            -PassThru `
            -WindowStyle Hidden

        if ($process.ExitCode -ne 0) {
            $failureCode = $process.ExitCode
            break
        }
    }
} finally {
    $env:PATH = $originalPath
    if ($null -eq $originalPlatform) {
        Remove-Item Env:QT_QPA_PLATFORM -ErrorAction SilentlyContinue
    } else {
        $env:QT_QPA_PLATFORM = $originalPlatform
    }
    if ($null -eq $originalScaleFactor) {
        Remove-Item Env:QT_SCALE_FACTOR -ErrorAction SilentlyContinue
    } else {
        $env:QT_SCALE_FACTOR = $originalScaleFactor
    }
    if ($null -eq $originalScreenScaleFactors) {
        Remove-Item Env:QT_SCREEN_SCALE_FACTORS -ErrorAction SilentlyContinue
    } else {
        $env:QT_SCREEN_SCALE_FACTORS = $originalScreenScaleFactors
    }
    if ($null -eq $originalSuppressWarnings) {
        Remove-Item Env:QTFLUENT_SUPPRESS_KNOWN_PAINT_WARNINGS -ErrorAction SilentlyContinue
    } else {
        $env:QTFLUENT_SUPPRESS_KNOWN_PAINT_WARNINGS = $originalSuppressWarnings
    }
}

if ($failureCode -ne 0) {
    exit $failureCode
}

$expected = $pageItems.Count * $variantItems.Count * $scaleItems.Count
if (-not (Test-RenderedPngHealth -Directory $resolvedOutput -Filter "*.png" -MinimumCount $expected)) {
    exit 1
}

$actual = (Get-ChildItem -LiteralPath $resolvedOutput -Filter "*.png" -File -ErrorAction SilentlyContinue | Measure-Object).Count
if ($actual -ne $expected) {
    Write-Error "Expected exactly $expected visual baseline images, captured $actual."
    exit 1
}
if (-not (Test-RenderedMatrixCompleteness -Directory $resolvedOutput -Pages $pageItems -VariantNames $variantOutputNames -ExpectedPerPageVariant $scaleItems.Count)) {
    exit 1
}

Write-Host "Rendered $actual visual baselines in $resolvedOutput"
exit 0
