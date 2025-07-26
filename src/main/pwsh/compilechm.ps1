#CompileChm.ps1
Param(
  [Parameter(Mandatory)]
  [string]$HtmlHelpProject,
  [Parameter(Mandatory)]
  [string]$Destination
)

# ファイル不存在チェック
# Path Is Missing と読めるように作成
function Is-Missing
{
  [CmdletBinding()]
  Param(
    [Parameter(Mandatory, ValueFromPipeline)]
    [string]$Path
  )

  return -not(Test-Path -Path $Path)
}

# ファイルロックチェック
# Path Is Locked と読めるように作成
function Is-Locked
{
  [CmdletBinding()]
  Param(
    [Parameter(Mandatory, ValueFromPipeline)]
    [string]$Path
  )

  if ($Path | Is-Missing) {
    return $false
  }

  try
  {
    $File = New-Object System.IO.FileInfo $Path
    $Stream = $File.Open(
      [System.IO.FileMode]::Open,
      [System.IO.FileAccess]::ReadWrite,
      [System.IO.FileShare]::None)

    return $false
  }
  catch [System.IO.IOException]
  {
    Write-Output "file is locked by a process."
    return $true
  }
  finally
  {
    if ($null -ne $Stream)
    {
      $Stream.Close()
    }
  }
}

# コンパイル済みHTMLのコピー処理
# Azure Pipelinesがたまにコピー失敗する対策として作成
function Copy-Chm
{
  [CmdletBinding()]
  Param(
    [Parameter(Mandatory)]
    [string]$Path,
    [Parameter(Mandatory)]
    [string]$Destination
  )

  if ($Path | Is-Missing)
  {
    return $false
  }

  if ($Destination.EndsWith([System.IO.Path]::DirectorySeparatorChar))
  {
    if ($Destination | Is-Missing)
    {
      New-Item -Path $Destination -ItemType Directory
    }

    $Destination = [System.IO.Path]::Combine($Destination, [System.IO.Path]::GetFileName($Path))
  }

  if ($Path -like $Destination)
  {
    Write-Output "Copy is not required."
    return $true
  }

  try
  {
    Write-Output "`$CompiledHelp is: $Path"
    Write-Output "`$Destination  is: $Destination"
    Copy-Item -Path $Path -Destination $Destination
  }
  catch #[System.IO.IOException]
  {
    Write-Output "Copy Chm error [$($PSItem.Exception)]: $($PSItem.ToString())"
    return $false
  }

  return $true
}

$HtmlHelpProject = Convert-Path $HtmlHelpProject

if (-not($HtmlHelpProject -imatch '\.hhp$'))
{
  throw [System.ArgumentException]::new("Bad filename of `$HtmlHelpProject: $HtmlHelpProject")
}

if ([string]::IsNullOrEmpty($env:CMD_HHC) -or ($env:CMD_HHC | Is-Missing))
{
  $env:CMD_HHC = 'C:\Program Files (x86)\HTML Help Workshop\hhc.exe'
}

$hhc = $env:CMD_HHC -replace '(.+)', '""$1""'
$CompiledHelp = $HtmlHelpProject -ireplace '\.hhp$', '.chm'
$CompileLog = "$([System.IO.Path]::GetDirectoryName($HtmlHelpProject))\Compile.Log"

if (Test-Path -Path $CompileLog)
{
  Remove-Item -Path $CompileLog
}

if (Test-Path -Path $CompiledHelp)
{
  Remove-Item -Path $CompiledHelp
}

if (Test-Path -Path $Destination)
{
  Remove-Item -Path $Destination
}

while ($true)
{
  try
  {
    if ([string]::IsNullOrEmpty($env:CMD_LEPROC))
    {
      #kick cmd.exe for a legacy command.
      Start-Process $env:CMD_HHC $HtmlHelpProject
    }
    else
    {
      #kick LEProc.exe for a legacy command.
      Start-Process $env:CMD_LEPROC "$env:COMSPEC /C `"$hhc $HtmlHelpProject`""
    }

    #wait for complete
    for ($count = 0; $count -lt 30; $count++)
    {
      if ($CompiledHelp | Is-Missing)
      {
        Write-Output "`$CompiledHelp is missing: $CompiledHelp"
      }
      elseif ($CompiledHelp | Is-Locked)
      {
        Write-Output "`$CompiledHelp is still locked: $CompiledHelp"
      }
      elseif ($CompileLog | Is-Missing)
      {
        Write-Output "`$CompileLog is missing: $CompileLog"
      }
      elseif ($CompileLog | Is-Locked)
      {
        Write-Output "`$CompileLog is still locked: $CompileLog"
      }
      elseif (Copy-Chm $CompiledHelp $Destination)
      {
        return
      }
      else
      {
        Write-Output "Copy `$CompiledHelp has failed"
        break
      }

      Start-Sleep -Second 1
    }
  }
  catch [System.AccessViolationException]
  {
    Write-Output "LEProc.exe has crashed"
  }
}
