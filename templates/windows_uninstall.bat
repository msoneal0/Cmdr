$app_target -stop
schtasks /delete /f /tn $app_name
del /f "%windir%\$app_target.exe"
del /f "%SystemDrive%\ProgramData\Microsoft\Windows\Start Menu\Programs\$app_name.lnk"
rd /q /s "$install_dir"
echo "Uninstallation Complete"