#!/usr/bin/env python3
from pathlib import Path
import json
from datetime import datetime

import click

from e_status_time import run_e
 

def check_path(ctx, param, path):
    if not Path(path).exists():
        raise click.BadParameter(F"Path '{path}' does not exist!")
    return path


def run_test(binary, filepath):
    args = [binary, 
            "--auto",
            "--memory-limit=2048",
            "--cpu-limit=180",
            "--silent",
            str(filepath)]
    res = run_e(args)
    return (res[0], res[1])


@click.command()
@click.option("--binary", 
              default="../PROVER/eprover",
              callback=check_path,
              help="Path to eprover binary.")
@click.option("--log-dir",
              default=".",
              callback=check_path,
              help="Path to directory in which to save logfiles.")
@click.option("--problems",
              default="../EXAMPLE_PROBLEMS/SMOKETEST",
              callback=check_path,
              help="Path to problem sets.")
@click.option("--branch",
              default="main",
              help="Git branch.")
@click.option("--count",
              default=1,
              help="Number of runs per file.")
def main(binary, log_dir, problems, branch, count):
    types = ["*.p", "*.lop"]
    log =  {}
    stamp = datetime.now().strftime("%H%M_%d%m%Y")

    log_dir = Path(log_dir).joinpath(branch)
    if not log_dir.exists():
        log_dir.mkdir()
    log_file = log_dir.joinpath(F"results{stamp}.json")

    file_list = [problem for t in types
                         for problem in Path(problems).rglob(t)]
    
    with click.progressbar(file_list) as bar:
        for problem in bar:
            print("\t", problem)
            problem = Path(problem)
            name = problem.name

            if name not in log:
                log.update({name: {"time": 0, "time_mean": 0, "runs": {}}})
            
            for run in range(0, count):
                res, duration = run_test(binary, problem)
                log[name]["time"] += duration
                log[name]["runs"].update({run: {"result": res, "time": duration}})
            log[name]["time_mean"] = log[name]["time"] / count
   
    # Check if all results equal
    for k, v in log.items():
        for _, run in v["runs"].items():
            if run["result"] != v["runs"][0]["result"]:
                print("NOT EQUAL")

    with open(log_file, "w") as outfile:
        json.dump(log, outfile, indent=4)


if __name__ == "__main__":
    main()
