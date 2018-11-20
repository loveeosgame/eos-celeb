
eosiocpp -g eoscrazytown/eoscrazytown.abi  eoscrazytown/eoscrazytown.cpp &eosiocpp -o eoscrazytown/eoscrazytown.wast eoscrazytown/eoscrazytown.cpp

cleos set contract eosbocaihero eoscrazytown -p eosbocaihero

cleos push action eosbocaihero init '[]' -p eosbocaihero 

cleos push action eosbocaihero newbag '["1.0000 EOS"]' -p eosbocaihero 

