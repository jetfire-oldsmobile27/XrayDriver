$dirs = @("include", "src")
$subdirs = @("service", "driver")
$patterns = @("*.cpp", "*.h", "*.hpp", "*.c", "*.cxx", "*.hxx", "*.cmake", "*.make", "*.json")

# Абсолютный путь к корню проекта (текущая директория)
$projectRootPath = (Get-Location).ProviderPath

foreach ($subdir in $subdirs) {
    foreach ($dir in $dirs) {
        $path = Join-Path $dir $subdir
        if (Test-Path -Path $path -PathType Container) {
            $files = Get-ChildItem -Path $path -Recurse -Include $patterns -File
            foreach ($file in $files) {
                # Относительный путь от корня проекта
                $relativePath = $file.FullName.Substring($projectRootPath.Length + 1)
                # Выводим с явным $() вокруг переменной
                Write-Output "$($relativePath):"
                Get-Content -Path $file.FullName -Raw
                Write-Output ""
            }
        }
    }
}
