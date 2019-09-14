#!/bin/sh

qt_dir="$1"
installer_file="$2"

src_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
bin_name="cmdr"
app_version="1.0.0"
app_name="Cmdr"
install_dir="/opt/$bin_name"
bin_dir="/usr/bin"
tmp_dir="$HOME/.cache/cmdr_build"

if [ "$qt_dir" != "" ]; then

 PATH=$qt_dir:$PATH
 
fi

if [ "$installer_file" = "" ]; then
 
 installer_file="$src_dir/$bin_name-$app_version.run"
  
fi
 
if [ -d "$tmp_dir" ]; then
 
 rm -rfv $tmp_dir
  
fi
 
mkdir -vp $tmp_dir
cp -rv $src_dir/. $tmp_dir
 
if [ $? -eq 0 ]; then
 
  cd $tmp_dir
 
  qmake -config release
 
  if [ $? -eq 0 ]; then
   
   make
   
   if [ $? -eq 0 ]; then
     
    mkdir -v ./build
    mkdir -v ./build/lib
    cp -rfv ./icons ./build
    ldd ./$bin_name | grep "libQt" | awk '{print $3}' | xargs -I '{}' cp -v '{}' ./build/lib
    ldd ./$bin_name | grep "libicu" | awk '{print $3}' | xargs -I '{}' cp -v '{}' ./build/lib
    ldd ./$bin_name | grep "libssl" | awk '{print $3}' | xargs -I '{}' cp -v '{}' ./build/lib
    ldd ./$bin_name | grep "libcrypto" | awk '{print $3}' | xargs -I '{}' cp -v '{}' ./build/lib
    mv -v ./$bin_name ./build/$bin_name
    
    startup_script="./build/$bin_name.sh"
    setup_script="./build/setup.sh"
    uninstall_script="./build/uninstall.sh"
    desktop_file="./build/$bin_name.desktop"
    
    echo "#!/bin/sh" > $startup_script
    echo "export LD_LIBRARY_PATH=$install_dir/lib" >> $startup_script
    echo "$install_dir/$bin_name \$1 \$2 \$3 \$4" >> $startup_script
    
    echo "[Desktop Entry]" > $desktop_file
    echo "Type=Application" >> $desktop_file
    echo "Exec=$bin_dir/$bin_name" >> $desktop_file
    echo "Name=$app_name" >> $desktop_file
    echo "GenericName=Terminal emulator for MRCI host." >> $desktop_file
    echo "Icon=$bin_name" >> $desktop_file
    echo "StartupWMClass=$bin_name" >> $desktop_file
    echo "Terminal=false" >> $desktop_file
    echo "Categories=Network;Terminal;MRCI;" >> $desktop_file
   
    echo "#!/bin/sh" > $setup_script
    echo "if [ -f \"$install_dir/uninstall.sh\" ]; then" >> $setup_script
    echo " sh $install_dir/uninstall.sh" >> $setup_script
    echo "fi" >> $setup_script
    echo "if [ ! -d \"$install_dir\" ]; then" >> $setup_script
    echo " sudo mkdir -p $install_dir" >> $setup_script
    echo "fi" >> $setup_script
    echo "sudo cp -rv ./lib $install_dir" >> $setup_script
    echo "sudo cp -v ./$bin_name $install_dir" >> $setup_script
    echo "sudo cp -v ./$bin_name.sh $install_dir" >> $setup_script
    echo "sudo cp -v ./uninstall.sh $install_dir" >> $setup_script
    echo "sudo chmod 755 $install_dir/$bin_name" >> $setup_script
    echo "sudo chmod 755 $install_dir/$bin_name.sh" >> $setup_script
    echo "sudo chmod 755 $install_dir/uninstall.sh" >> $setup_script
    echo "sudo chmod 755 $install_dir" >> $setup_script
    echo "sudo chmod -R 755 $install_dir/lib" >> $setup_script
     
    echo "sudo mkdir -p /usr/share/icons/hicolor/8x8/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/16x16/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/22x22/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/24x24/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/32x32/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/36x36/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/42x42/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/48x48/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/64x64/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/72x72/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/96x96/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/128x128/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/192x192/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/256x256/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/512x512/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/icons/hicolor/scalable/apps" >> $setup_script
    echo "sudo mkdir -p /usr/share/applications" >> $setup_script
     
    echo "sudo cp -v ./icons/8x8.png /usr/share/icons/hicolor/8x8/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/16x16.png /usr/share/icons/hicolor/16x16/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/22x22.png /usr/share/icons/hicolor/22x22/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/24x24.png /usr/share/icons/hicolor/24x24/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/32x32.png /usr/share/icons/hicolor/32x32/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/36x36.png /usr/share/icons/hicolor/36x36/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/42x42.png /usr/share/icons/hicolor/42x42/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/48x48.png /usr/share/icons/hicolor/48x48/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/64x64.png /usr/share/icons/hicolor/64x64/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/72x72.png /usr/share/icons/hicolor/72x72/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/96x96.png /usr/share/icons/hicolor/96x96/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/128x128.png /usr/share/icons/hicolor/128x128/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/192x192.png /usr/share/icons/hicolor/192x192/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/256x256.png /usr/share/icons/hicolor/256x256/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/512x512.png /usr/share/icons/hicolor/512x512/apps/$bin_name.png" >> $setup_script
    echo "sudo cp -v ./icons/scalable.svg /usr/share/icons/hicolor/scalable/apps/$bin_name.svg" >> $setup_script
    echo "sudo cp -v ./$bin_name.desktop /usr/share/applications/$bin_name.desktop" >> $setup_script
    
    echo "sudo chmod 644 /usr/share/icons/hicolor/8x8/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/16x16/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/22x22/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/24x24/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/32x32/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/36x36/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/42x42/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/48x48/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/64x64/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/72x72/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/96x96/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/128x128/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/192x192/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/256x256/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/512x512/apps/$bin_name.png" >> $setup_script
    echo "sudo chmod 644 /usr/share/icons/hicolor/scalable/apps/$bin_name.svg" >> $setup_script
    echo "sudo chmod 644 /usr/share/applications/$bin_name.desktop" >> $setup_script
    
    echo "sudo ln -sfv $install_dir/$bin_name.sh $bin_dir/$bin_name" >> $setup_script
    echo "echo \"\nInstallation finished. If you ever need to uninstall this application, run this command:\n\"" >> $setup_script
    echo "echo \"   sh $install_dir/uninstall.sh\n\"" >> $setup_script
     
    echo "#!/bin/sh" > $uninstall_script
    echo "sudo rm -v $bin_dir/$bin_name" >> $uninstall_script
    echo "sudo rm -rv $install_dir" >> $uninstall_script
     
    echo "sudo rm -v /usr/share/icons/hicolor/8x8/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/16x16/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/22x22/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/24x24/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/32x32/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/36x36/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/42x42/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/48x48/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/64x64/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/72x72/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/96x96/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/128x128/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/192x192/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/256x256/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/512x512/apps/$bin_name.png" >> $uninstall_script
    echo "sudo rm -v /usr/share/icons/hicolor/scalable/apps/$bin_name.svg" >> $uninstall_script
    echo "sudo rm -v /usr/share/applications/$bin_name.desktop" >> $uninstall_script
     
    echo "echo Finished." >> $uninstall_script
    
    chmod +x $setup_script
    
    makeself ./build $installer_file "$app_name Installation" ./setup.sh
    
   fi
   
 fi
 
fi

if [ -d "$tmp_dir" ]; then
 
 rm -rf $tmp_dir
  
fi