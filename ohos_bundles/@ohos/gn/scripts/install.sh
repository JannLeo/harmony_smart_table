HPM_FILE_NAME="gn.tar"
HPM_TOOLS_LINUX_DOWNLOAD_URL="https://repo.huaweicloud.com/harmonyos/compiler/gn/1523/linux/gn.1523.tar"
HPM_TOOLS_WINDOWS_DOWNLOAD_URL=""
HPM_TOOLS_DOWNLOAD_URL=${HPM_TOOLS_WINDOWS_DOWNLOAD_URL}


if [ "$(uname)" = "Darwin" ]; then
    HPM_TOOLS_DOWNLOAD_URL=${HPM_TOOLS_LINUX_DOWNLOAD_URL}
elif [ "$(uname)" = "Linux" ]; then
    HPM_TOOLS_DOWNLOAD_URL=${HPM_TOOLS_LINUX_DOWNLOAD_URL}
fi

# HPM_TOOLS_DOWNLOAD_URL 镜像上的包下载地址  HPM_FILE_NAME 下载文件名称
curl ${HPM_TOOLS_DOWNLOAD_URL} -o ${HPM_FILE_NAME} 

# 解压工具包到用户指定位置
tar -xvf ${HPM_FILE_NAME}
# 重试以防link文件首次找不到源文件
tar -xvf ${HPM_FILE_NAME}

echo  "gn : install success"
