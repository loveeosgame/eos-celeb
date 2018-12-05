
eosiocpp -g eoscrazytown/eoscrazytown.abi  eoscrazytown/eoscrazytown.cpp && eosiocpp -o eoscrazytown/eoscrazytown.wast eoscrazytown/eoscrazytown.cpp

cleos set contract eosbocaihero eoscrazytown -p eosbocaihero

cleos push action eosbocaihero init '[]' -p eosbocaihero 

cleos push action eosbocaihero newbag '["0.1000 EOS",0]' -p eosbocaihero 
cleos push action eosbocaihero newbag '["0.1000 EOS",57]' -p eosbocaihero 

