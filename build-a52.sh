echo -e "\nStarting compilation...\n"

# ENV
CONFIG=vendor/sixteen-a52q_defconfig
KERNEL_DIR=$(pwd)
PARENT_DIR="$(dirname "$KERNEL_DIR")"
KERN_IMG="$HOME/out-a52/out/arch/arm64/boot/Image.gz-dtb"
export KBUILD_BUILD_USER="elang"
export KBUILD_BUILD_HOST="kyvangkaelang"
export PATH="$HOME/toolchain/clang-r475365/bin:$PATH"
export LD_LIBRARY_PATH="$HOME/toolchain/clang-r475365/lib:$LD_LIBRARY_PATH"
export KBUILD_COMPILER_STRING="$($HOME/toolchain/clang-r475365/bin/clang --version | head -n 1 | perl -pe 's/\((?:http|git).*?\)//gs' | sed -e 's/  */ /g' -e 's/[[:space:]]*$//' -e 's/^.*clang/clang/')"
export out=$HOME/out-a52-ONEUI

# Functions
clang_build () {
    make -j$(nproc --all) O=$out \
                          ARCH=arm64 \
                          CC="clang" \
                          AR="llvm-ar" \
                          NM="llvm-nm" \
			              AS="llvm-as" \
                          LD="ld.lld" \
                          STRIP="llvm-strip" \
			              OBJCOPY="llvm-objcopy" \
			              OBJDUMP="llvm-objdump" \
                          CROSS_COMPILE="$HOME/toolchain/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-" \
	                      CROSS_COMPILE_ARM32="$HOME/toolchain/gcc-arm-11.2-2022.02-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-"
}

# Build kernel
make O=$out ARCH=arm64 $CONFIG > /dev/null
echo -e "${bold}Compiling with CLANG${normal}\n$KBUILD_COMPILER_STRING"
echo -e "\nCompiling $ZIPNAME\n"
clang_build
if [ -f "$out/arch/arm64/boot/Image.gz-dtb" ] && [ -f "$out/arch/arm64/boot/dtbo.img" ]; then
 echo -e "\nKernel compiled succesfully! Zipping up...\n"
 ZIPNAME="SixTeen•Kernel•ONEUI•Samsung•A52-$(date '+%Y%m%d-%H%M').zip"
 if [ ! -d AnyKernel3 ]; then
  git clone -q https://github.com/Kyvangka1610/AnyKernel3.git -b samsung
 fi;
 mv -f $out/arch/arm64/boot/Image.gz-dtb AnyKernel3
 mv -f $out/arch/arm64/boot/dtbo.img AnyKernel3
 cd AnyKernel3
 zip -r9 "$HOME/$ZIPNAME" *
 cd ..
 rm -rf AnyKernel3
 echo -e "\nCompleted in $((SECONDS / 60)) minute(s) and $((SECONDS % 60)) second(s) !"
 echo -e "Zip: $ZIPNAME\n"
else
 echo -e "\nCompilation failed!\n"
fi;
