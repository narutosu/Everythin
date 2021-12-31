import shutil
for i in range(100):
    shutil.copy("pakchunk1007-WindowsNoEditor.pak","pakchunk1007-WindowsNoEditor_"+ bytes(i) +"_p.pak")