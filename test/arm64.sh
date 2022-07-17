# Run from the root dir.

gn clean out
./gn_gen.sh --target_arch arm64 --target_board qemu_virt --host gnu --test_method tests
cd out
./build.sh
./sync.sh
python3 ../utils/test/test.py
cd ..
gn clean out
./gn_gen.sh --target_arch arm64 --target_board qemu_virt --host llvm --test_method tests
cd out
./build.sh
./sync.sh
python3 ../utils/test/test.py
cd ..
gn clean out
