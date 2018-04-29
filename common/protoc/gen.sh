rm -rf ./gprotoc/
mkdir gprotoc
protoc internal_protocol.proto --cpp_out=./gprotoc/
protoc external_protocol.proto --cpp_out=./gprotoc/
