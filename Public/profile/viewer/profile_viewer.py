# 入口。界面在 main.py，三种读取在 read_file.py、read_pid.py、read_ws.py。
# 运行：python Public/profile/viewer/profile_viewer.py
# 自测：python Public/profile/viewer/profile_viewer.py --self-test

import sys

from main import main

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
