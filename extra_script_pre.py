Import("env")

# print("Current CLI targets", COMMAND_LINE_TARGETS)


def before_upload():
    print("[Extra-script]: Running before_upload()")

    import re

    version_file = "src/common/version.h"
    version_pattern = r'(^.*SW_VERSION\s*=\s*")(\d+)\.(\d+)\.(\d+)"'

    with open(version_file, "r") as file:
        content = file.readlines()

    with open(version_file, "w") as file:
        for line in content:
            match = re.search(version_pattern, line)
            if match:
                major_version = match.group(2)
                minor_version = match.group(3)
                patch_version = int(match.group(4))
                new_line = f'const char *SW_VERSION = "{major_version}.{minor_version}.{patch_version + 1}";\n'
                file.write(new_line)
            else:
                file.write(line)


# env.AddPreAction("upload", before_upload)
# env.AddPreAction("buildprog", before_upload)

before_upload()
