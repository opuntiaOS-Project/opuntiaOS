import os

ignore = [
    '.DS_Store'
]

apps_dir = 'src/apps'
apps_dir_full = os.getcwd() + '/' + apps_dir
apps_dir_len = len(apps_dir)
installer = './utils/app_installer.exec'
print(apps_dir)

folders = []
files = []

# r = root, d = directories, f = files
for r, d, f in os.walk(apps_dir):
    for folder in d:
        folders.append((r[apps_dir_len:]+'/', folder))
    for file in f:
        files.append((r[apps_dir_len:]+'/', file))

for path, fname in folders:
    cmd = "{0} mkdir {1} {2}".format(installer, path, fname)
    print(cmd)
    os.system(cmd);

for path, fname in files:
    print (apps_dir+'/'+fname);
    cmd = "{0} writefile {1} {2} {3}".format(installer, path, fname, apps_dir+'/'+fname)
    if not (fname in ignore):
        print(cmd)
        os.system(cmd);
