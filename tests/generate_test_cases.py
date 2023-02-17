import random


def generate_tests():
    file_name="" 
    tickers = ["A", "AA", "AAC", "AAIC", "AAIC^B", "AAIC^C", "AAIN", "AAM^A", "AAM^B", "AAN"]
    for i in range(100): 
        file_name="complex_"+str(i)+".in"
        f=open(file_name,'w').close()
        f = open(file_name,'a')
        f.write("40\n")
        f.write("o\n")
        id = 100
        for i in range(random.randint(1000,50000)):
            op = random.randint(1,2)
            client = random.randint(0, 39)
            if op == 1:
                f.write(str(client) + " B " + str(id) + " " + tickers[random.randint(0,len(tickers)-1)] + " " + str(random.randint(100,2000)) + " " + str(random.randint(10,1000)) + "\n" )
                id += 1
            elif op == 2:
                f.write(str(client) + " S " + str(id) + " " + tickers[random.randint(0,len(tickers)-1)] + " " + str(random.randint(100,2000)) + " " + str(random.randint(10,1000)) + "\n" )
                id += 1
        f.write("x\n")
        f.close()
            
            
generate_tests()