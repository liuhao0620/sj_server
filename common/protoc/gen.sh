rm -rf ./gprotoc/
mkdir gprotoc
ls *.proto | xargs -iPATH protoc PATH --cpp_out=./gprotoc/
