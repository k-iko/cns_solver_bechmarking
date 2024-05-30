import sys
import argparse
import pandas as pd
import plotly
from plotly import express as px
from plotly import graph_objects as go

def main(arguments):
    parser = argparse.ArgumentParser()
    parser.add_argument('resultFile', help="result file", type=str)
    parser.add_argument('demandFile', help="demand file", type=str)
    parser.add_argument('outputFile', help="output file", type=str)
    args = parser.parse_args(arguments)

    if args.resultFile and args.demandFile and args.outputFile:
        df_demand = pd.read_csv(args.demandFile, index_col=0)
        df_result = pd.read_csv(args.resultFile, names=range(len(df_demand)+1), skiprows=[0], index_col=0)

        df_demand['DEMAND_plot'] = df_demand.DEMAND + 1

        fig = px.scatter(df_demand, x='LATITUDE', y='LONGITUDE', size='DEMAND_plot', color='DEMAND')

        for route in df_result.index:
            cols = ~df_result.loc[route].isna()
            df = df_demand.loc[df_result.loc[route, cols]]
            fig = fig.add_trace(go.Scatter(x=df.LATITUDE, y=df.LONGITUDE, mode='lines+markers', name=route))

        plotly.offline.plot(
            fig,
            filename = args.outputFile,
            auto_open=False)

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))


