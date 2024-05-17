# James
import os
import sys
import shutil


def roundup_bigthan1(val, valStr):
    #print('roundup_bigthan1 1 valStr=',valStr)
    tmp = val
    if tmp >= 1:
        return
    else:
        tmp = tmp * 10
        #print('roundup_bigthan1 2 str(float(tmp))=',str(float(tmp)))
        valStr.append(str(int(tmp)))
        #print('roundup_bigthan1 3 valStr=',valStr)
        roundup_bigthan1(tmp, valStr)
    
    
def mkdir(dirPath):
    if sys.version_info[0] < 3 and sys.version_info[1] < 2:
        if not os.path.exists(dirPath):
            os.makedirs(dirPath)  
    else:
        os.makedirs(dirPath, exist_ok=True)     

def mkfiledir(filePath):
    if sys.version_info[0] < 3 and sys.version_info[1] < 2:
        directory = os.path.dirname(filePath)
        if not os.path.exists(directory):
            os.makedirs(directory)  
    else:
        os.makedirs(os.path.dirname(filePath), exist_ok=True)        


def mycopyfile(src, dst):
    if os.path.isfile(src)==True:
        if not os.path.isdir(dst):
            os.makedirs(dst)
        if sys.version_info[0] < 3.2:
            try:
                shutil.copy2(src, dst)
            except IOError as io_err:
                os.makedirs(os.path.dirname(dst))
                shutil.copy2(src, dst)
        else:
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copy2(src, dst)
            

def mycopytree(src, dst, symlinks=False):
    names = os.listdir(src)
    if not os.path.isdir(dst):
        os.makedirs(dst)
          
    errors = []
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if symlinks and os.path.islink(srcname):
                linkto = os.readlink(srcname)
                os.symlink(linkto, dstname)
            elif os.path.isdir(srcname):
                shutil.copytree(srcname, dstname, symlinks)
            else:
                if os.path.isdir(dstname):
                    os.rmdir(dstname)
                elif os.path.isfile(dstname):
                    os.remove(dstname)
                shutil.copy2(srcname, dstname)
            # XXX What about devices, sockets etc.?
        except (IOError, os.error) as why:
            errors.append((srcname, dstname, str(why)))
        # catch the Error from the recursive copytree so that we can
        # continue with other files
        except OSError as err:
            errors.extend(err.args[0])
    try:
        shutil.copystat(src, dst)
    except WindowsError:
        # can't copy file access times on Windows
        pass
    except OSError as why:
        errors.extend((src, dst, str(why)))
#    if errors:
#        raise Error(errors)
