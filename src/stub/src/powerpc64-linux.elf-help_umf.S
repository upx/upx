  .section HUMF_A, "ax"
    b upx_mmap_and_fd_android

  .section HUMF_L, "ax"
    b upx_mmap_and_fd_linux
