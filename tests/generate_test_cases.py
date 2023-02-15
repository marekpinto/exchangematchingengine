import random


def generate_tests():
    file_name="" 
    tickers = ["A", "AA", "AAC", "AAIC", "AAIC^B", "AAIC^C", "AAIN", "AAM^A", "AAM^B", "AAN", "AAP", "AAT", "AB", "ABB", "ABBV", "ABC", "ABEV", "ABG", "ABM", "ABR", "ABR^D", "ABR^E", "ABR^F", "ABT", "AC", "ACA", "ACCO", "ACDI", "ACEL", "ACHR", "ACI", "ACM", "ACN", "ACP", "ACP^A", "ACR", "ACR^C", "ACR^D", "ACRE", "ACV", "ADC", "ADC^A", "ADCT", "ADM", "ADNT", "ADT", "ADX", "AEE", "AEFC", "AEG", "AEL", "AEL^A", "AEL^B", "AEM", "AENZ", "AEO", "AER", "AES", "AESC", "AEVA", "AFB", "AFG", "AFGB", "AFGC", "AFGD", "AFGE", "AFL", "AFT", "AFTR", "AG", "AGAC", "AGCO", "AGD", "AGI", "AGL", "AGM", "AGM^C", "AGM^D", "AGM^E", "AGM^F", "AGM^G", "AGO", "AGR", "AGRO", "AGS", "AGTI", "AGX", "AHH", "AHH^A", "AHL^C", "AHL^D", "AHL^E", "AHT", "AHT^D", "AHT^F", "AHT^G", "AHT^H", "AHT^I", "AI", "AIC", "AIF", "AIG", "AIG^A", "AIN", "AIO", "AIR", "AIRC", "AIT", "AIU", "AIV", "AIZ", "AIZN", "AJG", "AJRD", "AJX", "AJXA", "AKA", "AKO/A", "AKO/B", "AKR", "AL", "AL^A", "ALB", "ALC", "ALCC", "ALE", "ALEX", "ALG", "ALIT", "ALK", "ALL", "ALL^B", "ALL^G", "ALL^H", "ALL^I", "ALLE", "ALLG", "ALLY", "ALSN", "ALTG", "ALTG^A", "ALV", "ALX", "AM", "AMAM", "AMBC", "AMBP", "AMC", "AMCR", "AME", "AMG", "AMH", "AMH^G", "AMH^H", "AMK", "AMN", "AMOV", "AMP", "AMPS", "AMPX", "AMPY", "AMR", "AMRC", "AMRX", "AMT", "AMTD", "AMWL", "AMX", "AN", "ANAC", "ANET", "ANF", "ANVS", "AOD", "AOMR", "AON", "AORT", "AOS", "AP", "APAM", "APCA", "APD", "APE", "APG", "APGB", "APH", "APLE", "APO", "APRN", "APTV", "APTV^A", "AQN", "AQNA", "AQNB", "AQNU", "AQUA", "AR", "ARC", "ARCH", "ARCO", "ARDC", "ARE", "ARES", "ARGD", "ARGO", "ARGO^A", "ARI", "ARIS", "ARL", "ARLO", "ARMK", "ARNC", "AROC", "ARR", "ARR^C", "ARW", "ASA", "ASAI", "ASAN", "ASB", "ASB^E", "ASB^F", "ASC", "ASG", "ASGI", "ASGN", "ASH", "ASIX", "ASPN", "ASR", "ASX", "ATAQ", "ATCO", "ATCO^D", "ATCO^H", "ATCO^I", "ATEK", "ATEN", "ATGE", "ATH^A", "ATH^B", "ATH^C", "ATH^D", "ATH^E", "ATHM", "ATI", "ATIP", "ATKR", "ATO", "ATR", "ATTO", "ATUS", "AU", "AUB", "AUB^A", "AUD", "AUY", "AVA", "AVAL", "AVB", "AVD", "AVK", "AVNS", "AVNT", "AVTR", "AVY", "AVYA", "AWF", "AWI", "AWK", "AWP", "AWR", "AX", "AXAC", "AXL", "AXP", "AXR", "AXS", "AXS^E", "AXTA", "AYI", "AYX", "AZEK", "AZO", "AZRE", "AZUL", "AZZ", "B", "BA", "BABA", "BAC", "BAC^B", "BAC^E", "BAC^K", "BAC^L", "BAC^M", "BAC^N", "BAC^O", "BAC^P", "BAC^Q", "BAC^S", "BACA", "BAH", "BAK", "BALL", "BALY", "BAM", "BANC", "BAP", "BARK", "BAX", "BB", "BBAI", "BBAR", "BBD", "BBDC", "BBDO", "BBLN", "BBN", "BBU", "BBUC", "BBVA", "BBW", "BBWI", "BBY", "BC", "BC^A", "BC^B", "BC^C", "BCAT", "BCC", "BCE", "BCH", "BCO", "BCS", "BCSF", "BCX", "BDC", "BDJ", "BDN", "BDX", "BDXB", "BE", "BEDU", "BEKE", "BEN", "BEP", "BEP^A", "BEPC", "BEPH", "BEPI", "BERY", "BEST", "BF/A", "BF/B", "BFAC", "BFAM", "BFH", "BFK", "BFLY", "BFS", "BFS^D", "BFS^E", "BFZ", "BG", "BGB", "BGH", "BGR", "BGS", "BGSF", "BGSX", "BGT", "BGX", "BGY", "BH", "BHC", "BHE", "BHG", "BHIL", "BHK", "BHLB", "BHP", "BHR", "BHR^B", "BHR^D", "BHV", "BHVN", "BIG", "BIGZ", "BILL", "BIO", "BIO/B", "BIP", "BIP^A", "BIP^B", "BIPC", "BIPH", "BIPI", "BIT", "BJ", "BK", "BKD", "BKDT", "BKE", "BKH", "BKI", "BKKT", "BKN", "BKSY", "BKT", "BKU", "BLCO", "BLD", "BLDR", "BLE", "BLK", "BLND", "BLUA", "BLW", "BLX", "BMA", "BMAC", "BME", "BMEZ", "BMI", "BML^G", "BML^H", "BML^J"]
    for i in range(100): 
        file_name="complex_"+str(i)+".in"
        f=open(file_name,'w').close()
        f = open(file_name,'a')
        f.write("40\n")
        f.write("o\n")
        id = 100
        for i in range(random.randint(1000,50000)):
            id += 1
            op = random.randint(1,3)
            client = random.randint(0, 39)
            if op == 1:
                f.write(str(client) + " B " + str(id) + " " + tickers[random.randint(0,len(tickers)-1)] + " " + str(random.randint(100,2000)) + " " + str(random.randint(10,1000)) + "\n" )
            elif op == 2:
                f.write(str(client) + " S " + str(id) + " " + tickers[random.randint(0,len(tickers)-1)] + " " + str(random.randint(100,2000)) + " " + str(random.randint(10,1000)) + "\n" )
            elif op == 3:
                f.write(str(client) + " C " + str(random.randint(100, id+1000)) + "\n")
        f.write("x\n")
        f.close()
            
            
generate_tests()