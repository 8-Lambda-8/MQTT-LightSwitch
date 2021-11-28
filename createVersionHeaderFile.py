import subprocess


def runBash(bashCommand):
    process = subprocess.Popen(bashCommand.split(), stdout=subprocess.PIPE)
    output, error = process.communicate()
    return output.decode("utf-8")


versionString = runBash("git log -1 --pretty=format:%h")

s = len(runBash("git diff --cached --numstat").split("\n"))-1
print(s)
if (s):  # staged
    versionString += " !s:" + str(s)

u = len(runBash("git diff --numstat").split("\n"))-1
print(u)
if (u):  # unStaged
    versionString += " !u:" + str(u)

print(versionString)

with open("src/version.h", "w") as file:
    file.write("#define __gitVersion__ \"{str}\"".format(str=versionString))
