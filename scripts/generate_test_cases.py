import random
print("test")
tickers = ["A", "AA", "AAC", "AAIC", "AAID", "AAIE", "AAIN", "AAM", "AAN", "AAO", "BDD","CAE","FEG","GEGS","HAHEOD","GLIN","STRA","GORN","VIDE","GORN","STRIPE","DOOR","CONC","THRONE"]
for i in range(100): 
    file_name="tests/mediumHard_"+str(i)+".in"
    f=open(file_name, 'w').close()
    f=open(file_name,'a')
    f.write("15\n")
    f.write("o\n")
    #print(file_name)
    id = 100
    map = [[] for i in range(15)]
    for i in range(random.randint(20,2000)):
        id += 1
        op = random.randint(1,3)
        client = random.randint(0, 14)
        if op == 1:
            f.write(str(client) + " B " + str(id) + " " + tickers[random.randint(0,len(tickers)-1)] + " " + str(random.randint(100,2000)) + " " + str(random.randint(10,1000)) + "\n" )
            map[client].append(id)

        elif op == 2:
            f.write(str(client) + " S " + str(id) + " " + tickers[random.randint(0,len(tickers)-1)] + " " + str(random.randint(100,2000)) + " " + str(random.randint(10,1000)) + "\n" )
            map[client].append(id)
        elif op == 3:
            if (len(map[client]) > 0):
                cancelId = random.choice(map[client])
                f.write(str(client) + " C " + str(cancelId) + "\n")
    f.write("x\n")
    f.close()

