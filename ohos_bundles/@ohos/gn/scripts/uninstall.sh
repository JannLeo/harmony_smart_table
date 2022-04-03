HPM_FILE_NAME="gn.tar"
# HPM_GN_INSTALL 用户可在hpmrc中配置的 gn安装地址 调用 卸载都依赖于该值

if [ ! ${HPM_GN_INSTALL} ]; then
    HPM_GN_INSTALL=${globalRepo}
fi

rm -rf ${HPM_GN_INSTALL}/gn   
rm -rf ${HPM_GN_INSTALL}/gn.exe
rm  ${HPM_FILE_NAME}

