:: clang compile for windows subsystem 
clang++ *.cpp -mwindows -municode -ld2d1 -luser32 -lgdi32 -ldinput8 -ldxguid -lwindowscodecs -lole32 -o D2DTEST.EXE