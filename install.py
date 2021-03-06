#!/usr/bin/python3

import os
import subprocess
import shutil
import platform
import sys
import zipfile
import binascii
import tempfile

def cd():
    current_dir = os.path.dirname(__file__)
    
    if current_dir != "":
        os.chdir(current_dir)
        
def get_default_install_dir(app_target, app_name):
    if platform.system() == "Linux":
        return "/opt/" + app_target
    
    else:
        return os.environ['ProgramFiles'] + "\\" + app_name
    
def get_default_installer_path(app_ver, app_name):
    return os.path.expanduser("~") + os.sep + app_name + "-" + app_ver + ".py"
        
def get_install_dir(app_target, app_name):
    path = get_default_install_dir(app_target, app_name)
    
    print("The default install directory is: " + path)
    
    while(True):
        ans = input("Do you want to change it? (y/n): ")
        
        if ans == "y" or ans == "Y":
            path = input("Enter a new install directory (leave blank to go back to the default): ")
            path = os.path.normpath(path)
            break
            
        elif ans == "n" or ans == "N":
            break
            
    if path == "":
        return get_default_install_dir(app_target, app_name)
    
    else:
        return path
    
def get_installer_path(app_ver, app_name):
    path = get_default_installer_path(app_ver, app_name)
    
    print("The built .py installer will placed here: " + path)
    
    while(True):
        ans = input("Do you want to change the path? (y/n): ")
        
        if ans == "y" or ans == "Y":
            path = input("Enter a new path (leave blank to go back to the default): ")
            path = os.path.normpath(path)
            break
            
        elif ans == "n" or ans == "N":
            break
            
    if path == "":
        return get_default_installer_path(app_ver, app_name)
    
    else:
        return path
    
def make_install_dir(path):
    try:
        if not os.path.exists(path):
            os.makedirs(path)

        return True
            
    except:
        print("Failed to create the install directory, please make sure you are runnning this script with admin rights.")

        return False

def replace_bin(binary, old_bin, new_bin, offs):
    while(True):
        try:
            index = binary.index(old_bin, offs)
            binary = binary[:index] + new_bin + binary[index + len(old_bin):]
        
        except ValueError:
            break
    
    return binary

def bin_sub_copy_file(src, dst, old_bin, new_bin, offs):
    binary = bytearray()
    
    with open(src, "rb") as rd_file:
        binary = rd_file.read()
        binary = replace_bin(binary, old_bin, new_bin, offs)
        
    with open(dst, "wb") as wr_file:
        wr_file.write(binary)
        
def text_sub_copy_file(src, dst, old_text, new_text, offs):
    bin_sub_copy_file(src, dst, old_text.encode("utf-8"), new_text.encode("utf-8"), offs)

def text_template_deploy(src, dst, install_dir, app_name, app_target):
    print("dep: " + dst)

    text_sub_copy_file(src, dst, "$install_dir", install_dir, 0)
    text_sub_copy_file(dst, dst, "$app_name", app_name, 0)
    text_sub_copy_file(dst, dst, "$app_target", app_target, 0)

def verbose_copy(src, dst):
    print("cpy: " + src + " --> " + dst)
    
    if os.path.isdir(src):
        files = os.listdir(src)
        
        if not os.path.exists(dst):
            os.makedirs(dst)
        
        for file in files:
            tree_src = src + os.path.sep + file
            tree_dst = dst + os.path.sep + file
            
            if os.path.isdir(tree_src):
                if not os.path.exists(tree_dst):
                    os.makedirs(tree_dst)
            
            verbose_copy(tree_src, tree_dst)
            
    else:
        shutil.copyfile(src, dst)
        
def verbose_create_symmlink(src, dst):
    print("lnk: " + src + " --> " + dst)
    
    if os.path.exists(dst):
        os.remove(dst)
    
    os.symlink(src, dst)

def local_install(app_target, app_name):
    if platform.system() == "Linux":
        if not os.path.exists("app_dir/linux"):
            print("An app_dir for the Linux platform could not be found.")
            
        else:
            install_dir = get_install_dir(app_target, app_name)
            
            if os.path.exists(install_dir + "/uninstall.sh"):
                subprocess.run([install_dir + "/uninstall.sh"])
            
            if make_install_dir(install_dir):
                if not os.path.exists("/var/opt/" + app_target):
                    os.makedirs("/var/opt/" + app_target)
            
                text_template_deploy("app_dir/linux/" + app_target + ".sh", install_dir + "/" + app_target + ".sh", install_dir, app_name, app_target)
                text_template_deploy("app_dir/linux/uninstall.sh", install_dir + "/uninstall.sh", install_dir, app_name, app_target)
                text_template_deploy("app_dir/linux/" + app_target + ".desktop", "/usr/share/applications/" + app_target + ".desktop", install_dir, app_name, app_target)
            
                for image_res in ["8x8", "16x16", "22x22", "24x24", "32x32", "36x36", "42x42", "48x48", "64x64", "192x192", "256x256", "512x512"]:
                    src = "app_dir/icons/" + image_res + ".png"
                    dst = "/usr/share/icons/hicolor/" + image_res
                    
                    if os.path.exists(dst):
                        verbose_copy(src, dst + "/apps/" + app_target + ".png")
                        
                        subprocess.run(["chmod", "644", dst + "/apps/" + app_target + ".png"])
                        
                verbose_copy("app_dir/icons/scalable.svg", "/usr/share/icons/hicolor/scalable/apps/" + app_target + ".svg")
                verbose_copy("app_dir/linux/" + app_target, install_dir + "/" + app_target)
                verbose_copy("app_dir/linux/lib", install_dir + "/lib")
                verbose_copy("app_dir/linux/platforms", install_dir + "/platforms")
                verbose_copy("app_dir/linux/xcbglintegrations", install_dir + "/xcbglintegrations")
            
                verbose_create_symmlink(install_dir + "/" + app_target + ".sh", "/usr/bin/" + app_target)
                
                subprocess.run(["chmod", "644", "/usr/share/icons/hicolor/scalable/apps/" + app_target + ".svg"])
                subprocess.run(["chmod", "755", install_dir + "/" + app_target + ".sh"])
                subprocess.run(["chmod", "755", install_dir + "/" + app_target])
                subprocess.run(["chmod", "755", install_dir + "/uninstall.sh"])
            
                print("Installation finished. If you ever need to uninstall this application, run this command with root rights:")
                print("    sh " + install_dir + "/uninstall.sh\n")
            
    elif platform.system() == "Windows":
        if not os.path.exists("app_dir\\windows"):
            print("An app_dir for the Windows platform could not be found.")
            
        else:
            install_dir = get_install_dir(app_target, app_name)

            if os.path.exists(install_dir + "\\uninstall.bat"):
                subprocess.run([install_dir + "\\uninstall.bat"])

            if os.path.exists(install_dir):
                # this block is here to make sure the install_dir is deleted 
                # if/when the uninstall.bat fails to do so. in my test machine, 
                # the .bat script will delete install_dir if run directly but 
                # not when called through subprocess.run() for some reason.
                shutil.rmtree(install_dir)    

            if make_install_dir(install_dir):
                verbose_copy("app_dir\\windows", install_dir)
                verbose_copy("app_dir\\icons\\win_icon.ico", install_dir + "\\icon.ico")

                text_template_deploy("app_dir\\windows\\uninstall.bat", install_dir + "\\uninstall.bat", install_dir, app_name, app_target)
                text_template_deploy("app_dir\\windows\\icon.vbs", install_dir + "\\icon.vbs", install_dir, app_name, app_target)
                text_sub_copy_file(install_dir + "\\icon.vbs", install_dir + "\\icon.vbs", "%SystemDrive%", os.environ['SystemDrive'], 0)

                os.system("\"" + install_dir + "\\icon.vbs\"")

                print("Installation finished. If you ever need to uninstall this application, run this batch file with admin rights:")
                print("    " + install_dir + "\\uninstall.bat\n")
        
    else:
        print("The platform you are running in is not compatible.")
        print("  output from platform.system() = " + platform.system())
    
def dir_tree(path):
    
    ret = []
    
    if os.path.isdir(path):
        for entry in os.listdir(path):
            full_path = os.path.join(path, entry)
            
            if os.path.isdir(full_path):
                for sub_dir_file in dir_tree(full_path):
                    ret.append(sub_dir_file)
                    
            else:
                ret.append(full_path)
                
    return ret

def to_hex(data):
    return str(binascii.hexlify(data))[2:-1]

def from_hex(text_line):
    return binascii.unhexlify(text_line)
        
def make_install(app_ver, app_name):
    path = get_installer_path(app_ver, app_name)
    
    with zipfile.ZipFile("app_dir.zip", "w", compression=zipfile.ZIP_DEFLATED) as zip_file:
        print("Compressing app_dir --")
        
        for file in dir_tree("app_dir"):
            print("adding file: " + file)
            zip_file.write(file)
            
    text_sub_copy_file(__file__, path, "main(is_sfx=False)", "main(is_sfx=True)\n\n\n", 7700)
        
    with open(path, "a") as dst_file, open("app_dir.zip", "rb") as src_file:
        print("Packing the compressed app_dir into the sfx script file --")
        
        dst_file.write("# APP_DIR\n")
        
        stat = os.stat("app_dir.zip")
        
        while(True):
            buffer = src_file.read(4000000)
            
            if len(buffer) != 0:
                dst_file.write("# " + to_hex(buffer) + "\n")
                
                print(str(src_file.tell()) + "/" + str(stat.st_size))
            
            if len(buffer) < 4000000:
                break
                
    os.remove("app_dir.zip")
    
    print("Finished.")

def sfx():
    abs_sfx_path = os.path.abspath(__file__)
    mark_found = False
    
    os.chdir(tempfile.gettempdir())
    
    with open(abs_sfx_path) as packed_file, open("app_dir.zip", "wb") as zip_file:
        stat = os.stat(abs_sfx_path)
        
        print("Unpacking the app_dir compressed file from the sfx script.")
        
        while(True):
            line = packed_file.readline()
            
            if not line: 
                break
            
            elif mark_found:
                zip_file.write(from_hex(line[2:-1]))
                
                print(str(packed_file.tell()) + "/" + str(stat.st_size))
            
            else:
                if line == "# APP_DIR\n":
                    mark_found = True
                    
        print("Done.")
    
    if not mark_found:
        print("The app_dir mark was not found, unable to continue.")
        
    else:
        with zipfile.ZipFile("app_dir.zip", "r", compression=zipfile.ZIP_DEFLATED) as zip_file:
            print("De-compressing app_dir --")
            
            zip_file.extractall()
            
        print("Preparing for installation.")
        
        os.remove("app_dir.zip")
        
        with open("app_dir" + os.sep + "info.txt") as info_file:
            info = info_file.read().split("\n")
            
            local_install(info[0], info[2])
            shutil.rmtree("app_dir") 

def main(is_sfx):
    cd()
    
    app_target = ""
    app_ver = ""
    app_name = ""
    
    if not is_sfx:
        with open("app_dir" + os.sep + "info.txt") as info_file:
            info = info_file.read().split("\n")
            
            app_target = info[0]
            app_ver = info[1]
            app_name = info[2]

    if is_sfx:
        sfx()
    
    elif "-local" in sys.argv:
        local_install(app_target, app_name)
        
    elif "-installer" in sys.argv:
        make_install(app_ver, app_name)
        
    else:
        print("Do you want to install onto this machine or create an installer?")
        print("[1] local machine")
        print("[2] create installer")
        print("[3] exit")
        
        while(True):
            opt = input("select an option: ")
            
            if opt == "1":
                local_install(app_target, app_name)
                break
                
            elif opt == "2":
                make_install(app_ver, app_name)
                break
                
            elif opt == "3":
                break

if __name__ == "__main__":
    main(is_sfx=False)