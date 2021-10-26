import json
import csv
import random

def getRandomShares(data_path, count = 100):
    selected_shares = []
    meta_path = data_path + "/meta.csv"
    close_path = data_path + "/close.csv"
    returns_path = data_path + "/returns.csv"

    meta_file = open(meta_path, newline='')
    close_file = open(close_path, newline='')
    returns_file = open(returns_path, newline='')

    meta_reader = csv.reader(meta_file)
    close_header = csv.DictReader(close_file).fieldnames
    returns_header = csv.DictReader(returns_file).fieldnames

    rows_list = list(meta_reader)
    
    i = 0
    while i < count:
        share_name = random.choice(rows_list)[0]
        if ((share_name in close_header) and (share_name in returns_header) and 
                (share_name not in selected_shares)):
            selected_shares.append(share_name)
            i = i + 1

    meta_file.close()
    close_file.close()
    returns_file.close()
    return selected_shares


def generateJSON(data_path, shares_num):
    data = {}
    data["rqId"] = "gen_test"
    data["model"] = "mad_min"
    data["market"] = "USA"
    data["currency"] = "USD"
    data["universe"] = getRandomShares(data_path, shares_num)

    data["T_0"] = "2015-06-01"
    data["T_1"] = "2021-04-16"
    data["parameters"] = {}
    data["parameters"]["max_risk"] = 0.01
    data["parameters"]["p_max"] = 0.15
    data["parameters"]["min_return"] = 0.0016
    data["parameters"]["t"] = 200 #1000
    data["parameters"]["capital"] = 10000000
    data["parameters"]["wmin"] = 0.9
    data["time_limit"] = 1800
    data["iteration_limit"] = 100
    data["node_limit"] = 200
    data["user_obj_limit"] = 200

    with open(data_path + "/input_params.json", 'w') as outfile:
        json.dump(data, outfile, indent=2)


if __name__ == "__main__":
    data_path = "../build/data"
    shares_num = 100
    generateJSON(data_path, shares_num)