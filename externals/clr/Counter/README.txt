to compile External.dll put  PureData.dll in this folder then execute this command:

mcs External.cs Counter.cs -out:External.dll -target:library -r:PureData.dll