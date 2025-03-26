# IoT AI-driven Yogurt Processing & Texture Prediction w/ Blynk
#
# Windows, Linux, or Ubuntu
#
# By Kutluhan Aktar
#
# Collect environmental factors and culture amount while processing yogurt. 
# Then, run a neural network model via Blynk to predict its texture.
#
#
# For more information:
# https://www.theamplituhedron.com/projects/IoT_AI_driven_Yogurt_Processing_Texture_Prediction

import numpy as np
import pandas as pd
from csv import writer

# Create a class to modify the given data set so as to upload properly formatted samples to Edge Impulse.
class process_dataset:
    def __init__(self, csv_path):
        # Read the data set from the given CSV file.
        self.df = pd.read_csv(csv_path)
        # Define the class (label) names.
        self.class_names = ["Thinner", "Optimum", "Curdling"]
    # Scale (normalize) data to define appropriately formatted inputs.
    def scale_data_elements(self):
        self.df["scaled_temperature"] = self.df["temperature"] / 100
        self.df["scaled_humidity"] = self.df["humidity"] / 100
        self.df["scaled_pressure"] = self.df["pressure"] / 1000
        self.df["scaled_milk_temperature"] = self.df["milk_temperature"] / 100
        self.df["scaled_starter_weight"] = self.df["starter_weight"] / 10
        print("Data Elements Scaled Successfully!")
    # Split the data set to generate a separate CSV file for each data record.    
    def split_dataset_by_labels(self, class_number):
        l = len(self.df)
        sample_number = 0
        # Split the data set according to the yogurt consistency levels (classes):
        for i in range(l):
            # Add the header as the first row:
            processed_data = [["temperature", "humidity", "pressure", "milk_temperature", "starter_weight"]]
            if(self.df["consistency_level"][i] == class_number):
                row = [self.df["scaled_temperature"][i], self.df["scaled_humidity"][i], self.df["scaled_pressure"][i], self.df["scaled_milk_temperature"][i], self.df["scaled_starter_weight"][i]]
                processed_data.append(row)
                # Increment the sample number:   
                sample_number+=1   
                # Create a CSV file for each data record, identified with the sample number.
                filename = "data/{}.sample_{}.csv".format(self.class_names[class_number], sample_number)
                with open(filename, "a", newline="") as f:
                    for r in range(len(processed_data)):
                        writer(f).writerow(processed_data[r])
                    f.close()
                print("CSV File Successfully Created: " + filename)
        
# Define a new class object named 'dataset':
dataset = process_dataset("yogurt_data.csv")

# Scale data and generate a separate CSV file for each sample:
dataset.scale_data_elements()
for c in range(len(dataset.class_names)):
    dataset.split_dataset_by_labels(c)
            
