import pandas as pd

df = pd.read_csv('all_info.csv')

all_groups = df.groupby(['length', 'size', 'file_num'])
new_data = pd.DataFrame(columns=['length', 'size', 'file_num', 'BHR_winner', 'BHRs', 'BHR_diff', 'OHR_winner', 'OHRs', 'OHR_diff'])
for group in all_groups:
    values = group[1]
    BHRs, OHRs = values['BHR'].values, values['OHR'].values
    BHR_diff = sorted(BHRs)[-1] - sorted(BHRs)[-2]
    OHR_diff = sorted(OHRs)[-1] - sorted(OHRs)[-2]
    BHR_line = values.loc[values['BHR'] == values['BHR'].max()]
    OHR_line = values.loc[values['OHR'] == values['OHR'].max()]
    line_to_df = [val for val in group[0]] + [BHR_line['cache'].values[0], BHRs, BHR_diff, OHR_line['cache'].values[0], OHRs, OHR_diff]
    add_df = pd.DataFrame([line_to_df], columns=new_data.columns)
    new_data = pd.concat([new_data, add_df], ignore_index=True)
new_data.to_csv('final_info.csv', index=False)
