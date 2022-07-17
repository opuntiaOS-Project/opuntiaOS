# Run from the root dir.

gn clean out
./gn_gen.sh -y --target_arch x86 --host gnu --test_method tests
cd out
./build.sh
./sync.sh
python3 ../utils/test/test.py
cd ..
gn clean out
./gn_gen.sh -y --target_arch x86 --host llvm --test_method tests
cd out
./build.sh
./sync.sh
python3 ../utils/test/test.py
cd ..
gn clean out
