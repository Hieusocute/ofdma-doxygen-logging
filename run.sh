echo "Log level = $1"
g++ main.cc destination.cc event.cc packet.cc transmission.cc -o main -lfmt
./main --log_level=$1 > output.txt
python3 plot.py
