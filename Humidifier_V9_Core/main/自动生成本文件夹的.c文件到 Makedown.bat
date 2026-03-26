@echo off
echo ==========================================
echo    🚀 Code Extractor (UTF-8 & GBK)
echo    正在提取 .c 和 .h 文件，请稍候...
echo ==========================================

set "PS_SCRIPT=%temp%\extract_code.ps1"

:: 内部逻辑全部采用纯英文写入，彻底断绝乱码导致语法崩溃的可能
echo $Output = 'all_code_summary.md' > "%PS_SCRIPT%"
echo Set-Content -Path $Output -Value '# Source Code Summary' -Encoding UTF8 >> "%PS_SCRIPT%"
echo $files = Get-ChildItem -Path . -Include *.c,*.h -Recurse >> "%PS_SCRIPT%"
echo $count = 0 >> "%PS_SCRIPT%"

echo foreach ($f in $files) { >> "%PS_SCRIPT%"
echo     Add-Content -Path $Output -Value "`n## File: $($f.FullName)`n" -Encoding UTF8 >> "%PS_SCRIPT%"
echo     Add-Content -Path $Output -Value "``````c" -Encoding UTF8 >> "%PS_SCRIPT%"
echo     try { >> "%PS_SCRIPT%"
echo         $text = Get-Content -Path $f.FullName -Encoding UTF8 -ErrorAction Stop >> "%PS_SCRIPT%"
echo     } catch { >> "%PS_SCRIPT%"
echo         $text = Get-Content -Path $f.FullName -Encoding Default >> "%PS_SCRIPT%"
echo     } >> "%PS_SCRIPT%"
echo     Add-Content -Path $Output -Value $text -Encoding UTF8 >> "%PS_SCRIPT%"
echo     Add-Content -Path $Output -Value "```````n" -Encoding UTF8 >> "%PS_SCRIPT%"
echo     $count++ >> "%PS_SCRIPT%"
echo     Write-Host "Extracted: $($f.Name)" >> "%PS_SCRIPT%"
echo } >> "%PS_SCRIPT%"

echo Write-Host "==========================================" >> "%PS_SCRIPT%"
echo Write-Host "Success! Total files extracted: $count" -ForegroundColor Green >> "%PS_SCRIPT%"
echo Write-Host "Saved to: all_code_summary.md" -ForegroundColor Green >> "%PS_SCRIPT%"

:: 运行脚本
powershell -NoProfile -ExecutionPolicy Bypass -File "%PS_SCRIPT%"
pause