import csv
from random import randrange, uniform
from datetime import datetime, timedelta

id_lots = ["ABRD", "ACKO", "AFKS", "AFLT", "AGRO",
"AKRN", "ALBK", "ALNU", "ALRS", "AMEZ", "APTK", "AQUA",
"ARSA", "ASSB", "AVAN","BANE", "BANEP", "BELU", "BISV","BISVP",
"BLNG"]

with open("lots.csv", mode="w") as w_file:
    top_names = ["SECID", "LOTSIZE"]
    file_writer = csv.DictWriter(w_file, delimiter = ",", 
                                 lineterminator="\r", fieldnames=top_names)
    file_writer.writeheader()
    for id in id_lots:
        random_size = randrange(1, 10000)
        file_writer.writerow({"SECID": id, "LOTSIZE": str(id)})


days_between = 400
today = datetime.today()
last_day = today - timedelta(days=days_between)
days_list = [(last_day + timedelta(days=x)).strftime('%Y-%m-%d') for x in range((today-last_day).days + 1)]

with open("close.csv", mode="w") as w_file:
    top_names = [""]
    top_names.extend(id_lots)
    file_writer = csv.DictWriter(w_file, delimiter = ",", 
                                 lineterminator="\r", fieldnames=top_names)
    file_writer.writeheader()
    for day in days_list:
        rand_close_dict = {lots: round(uniform(22, 100),3) for _,lots in enumerate(id_lots)}
        row = {"":day}
        row.update(rand_close_dict)
        file_writer.writerow(row)
