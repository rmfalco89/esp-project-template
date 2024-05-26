Import("env")

def after_upload(source, target, env):
    print("Opening serial monitor...")
    env.Execute("pio device monitor")

env.AddPostAction("upload", after_upload)
