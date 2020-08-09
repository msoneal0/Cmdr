set WshShell = CreateObject("Wscript.shell")
set oMyShortcut = WshShell.CreateShortcut("%SystemDrive%\ProgramData\Microsoft\Windows\Start Menu\Programs\$app_name.lnk")

oMyShortcut.IconLocation = "$install_dir\icon.ico"
OMyShortcut.TargetPath = "$install_dir\$app_target.exe"
oMyShortCut.Save