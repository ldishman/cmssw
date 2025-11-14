# Take one cabling map .csv and one "hacky" .csv

# Read hacky file: 
#   if (colF detId) !== (colA detId): 
#       if (colFDetId not in updateMap): 
#           updateMap[colFDetId] = []
#       updateMap[colFDetId].append(colADetId)

# Read cabling map and write output: 
#   if (cablingMap detId is in updateMap keys):
#       for entry in updateMap[cablingMapDetId]:
#           swap (cablingMap detId) with (hackyFile detId), write rest of line as-is to output.csv
#   else, write line as-is to output.csv

# Write output.csv to current directory


# FILE START
import csv

hacky_file = "hacky_noHeader.csv"
cabling_file = "cablingMap_noHeader.csv"
output_file = "hackedMap.csv"

updateMap = {}

# Read hacky file
with open(hacky_file, newline="") as f:
    reader = csv.reader(f)
    for row in reader:
        detA = row[0]   # colA
        detF = row[5]   # colF

        if detA != detF:
            if detF not in updateMap:
                updateMap[detF] = []
            updateMap[detF].append(detA)

# Read cabling map and write output
with open(cabling_file, newline="") as f_in, open(output_file, "w", newline="") as f_out:
    reader = csv.reader(f_in)
    writer = csv.writer(f_out)

    for row in reader:
        det = row[0]  # cabling map detId column

        if det in updateMap:
            for replacementDet in updateMap[det]:
                new_row = row[:]          # copy
                new_row[0] = replacementDet
                writer.writerow(new_row)
        else:
            writer.writerow(row)
