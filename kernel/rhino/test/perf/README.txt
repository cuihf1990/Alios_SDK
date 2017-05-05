1 Open the compile switch CONFIG_TEST_PERFORMANCE in the defconfig
2 In the root directory
  cp csp/vendor/csky/configs/defconfig_phobos_rhino ./defconfig
  cp build/core/Makefile ./
  cp id2kernel/build/yoc.mk id2kernel/
  make all
3 Performance test only support csky 802 mow.

