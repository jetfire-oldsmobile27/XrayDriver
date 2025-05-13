$dirs = @("include", "src")
$patterns = @("*.cpp", "*.h", "*.hpp", "*.c", "*.cxx", "*.hxx", "*.cmake", "*.make", "*.json")

foreach ($dir in $dirs) {
    if (Test-Path -Path $dir -PathType Container) {
        $files = Get-ChildItem -Path $dir -Recurse -Include $patterns -File
        foreach ($file in $files) {
            Write-Output "$($file.FullName):"
            Get-Content -Path $file.FullName -Raw
            Write-Output ""
        }
    }
}