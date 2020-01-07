#!/usr/bin/env python3
# -*- encoding: utf-8 -*-
import sys
import os
import re
import subprocess
import psutil
import json
import time
import signal
from datetime import datetime
from collections import defaultdict
from urllib import request

BENCHMARK_DIR = os.path.join(
	os.path.abspath(os.path.dirname(sys.argv[0])), "targets")
BENCHMARK_TARGETS = sorted(os.listdir(BENCHMARK_DIR))
BENCHMARK_CASES = [
	{
		"category": "throughput",
		"description": "request per second",
		"url": "http://127.0.0.1:8000",
		"commands": [
			{
				"name": "100 connections",
				"command": "wrk -c 100 -t 2 -d 60s --latency --timeout 100s %s"
			},
			{
				"name": "10000 connections",
				"command": "wrk -c 10000 -t 2 -d 120s --latency --timeout 100s %s"
			},
			{
				"name": "20000 connections",
				"command": "wrk -c 20000 -t 2 -d 180s --latency --timeout 100s %s"
			}
		],
		"result_regex": r"Requests/sec:\s*([\d\.]+)\n",
		"greater_is_better": True
	},
	{
		"category": "latency",
		"description": "the maximum response latency of 99% requests",
		"url": "http://127.0.0.1:8000",
		"commands": [
			{
				"name": "100 connections with 1000 rps",
				"command": "wrk2 -c 100 -t 2 -d 60s -R 1000 --latency --timeout 100s %s"
			},
			{
				"name": "10000 connections with 5000 rps",
				"command": "wrk2 -c 10000 -t 2 -d 120s -R 5000 --latency --timeout 100s %s"
			},
			{
				"name": "10000 connections with 10000 rps",
				"command": "wrk2 -c 10000 -t 2 -d 120s -R 10000 --latency --timeout 100s %s"
			}
		],
		"result_regex": r"99.000%\s*([\d\.]+[mun]?s)",
		"greater_is_better": False
	}
]
BENCHMARK_TASKSETS = [
	{ "name": "1 core", "server": "0", "client": "2,3" },
	{ "name": "2 cores", "server": "0,1", "client": "2,3" }
]
BEST_OF_N = 3
MARKDOWN_OUTPUT_PATH = "/tmp/cpv-benchmark-results.md"
ONLY_CPV_FRAMEWORK = True

class BenchmarkRunner(object):
	def __init__(self):
		self.fd = open(MARKDOWN_OUTPUT_PATH, "w", encoding="utf-8")
		print("Markdown will be written to: %s\n"%MARKDOWN_OUTPUT_PATH)
		self.write("# Benchmark results (%s)\n"%datetime.now().strftime("%Y-%m-%d"))
		self.write("benchmark environment: \n")
		self.write("- cpu model: %s"%subprocess.check_output(
			"cat /proc/cpuinfo | grep 'model name' | "
			"awk -F: '{ print $2 }' | head -n 1", shell=True).decode("utf-8").strip())
		self.write("- cpu cores: %s"%subprocess.check_output(
			"cat /proc/cpuinfo | grep 'cpu cores' | "
			"awk -F: '{ print $2 }' | head -n 1", shell=True).decode("utf-8").strip())
		self.write()
		self.write("benchmark options: \n")
		self.write("- best of n: %s"%BEST_OF_N)
		self.write()
	
	def write(self, line=""):
		print("\x1b[1;32m" + line + "\x1b[0m")
		if line:
			self.fd.write(line)
		self.fd.write("\n")
		self.fd.flush()
	
	def get_version(self, target):
		return (subprocess.check_output(
			"sh version.sh", cwd=os.path.join(BENCHMARK_DIR, target), shell=True)
			.decode("utf-8").strip())
	
	def build(self, target):
		p = subprocess.Popen(
			"sh build.sh", cwd=os.path.join(BENCHMARK_DIR, target), shell=True)
		if p.wait() != 0:
			raise RuntimeError("build %s error"%target)
	
	def wait_url_available(self, url, timeout=30):
		begin = time.time()
		code = None
		while True:
			try:
				code = request.urlopen(url).status
				if code == 200:
					return
			except Exception as e:
				if time.time() - begin > timeout:
					raise RuntimeError("wait url %s timeout"%url, e)
			if time.time() - begin > timeout:
				raise RuntimeError("url %s return status %s"%(url, code))
	
	def parse_result(self, result_obj):
		result_str = result_obj.group(1)
		if result_str.endswith("ms"):
			return float(result_str[:-2])
		elif result_str.endswith("us"):
			return float(result_str[:-2]) / 1000
		elif result_str.endswith("ns"):
			return float(result_str[:-2]) / 1000000
		elif result_str.endswith("s"):
			return float(result_str[:-1]) * 1000
		return float(result_str)
	
	def benchmark(self, target, case, taskset, command):
		self.write("##### %s / %s / %s\n"%(target, taskset["name"], command["name"]))
		command_server = "taskset -c %s sh run.sh"%taskset["server"]
		command_client = "taskset -c %s %s"%(
			taskset["client"], command["command"]%case["url"])
		self.write("run server with: `%s`\n"%command_server)
		self.write("run client with: `%s`\n"%command_client)
		best_result = None
		best_output = None
		try:
			proc_server = subprocess.Popen(
				command_server, cwd=os.path.join(BENCHMARK_DIR, target), shell=True)
			self.wait_url_available(case["url"])
			for n in range(BEST_OF_N):
				proc_client = subprocess.Popen(
					command_client, shell=True,
					stdout=subprocess.PIPE, stderr=subprocess.PIPE)
				outs, errs = proc_client.communicate()
				output = (outs + errs).decode("utf-8").strip()
				result_obj = re.search(case["result_regex"], output)
				if result_obj is None:
					raise RuntimeError("search result from output failed: %s"%output)
				result = self.parse_result(result_obj)
				print("round %d result: %.5f"%(n+1, result))
				if ((best_result is None) or
					(case["greater_is_better"] and result > best_result) or
					(not case["greater_is_better"] and result < best_result)):
					best_result = result
					best_output = output
		finally:
			for child in psutil.Process(proc_server.pid).children(recursive=True):
				try:
					os.kill(child.pid, signal.SIGINT)
				except ProcessLookupError:
					pass
			try:
				os.kill(proc_server.pid, signal.SIGINT)
			except ProcessLookupError:
				pass
			print("server exit with: %d"%proc_server.wait())
		self.write("output:\n")
		self.write("``` text")
		self.write(best_output)
		self.write("```\n")
		return best_result
	
	def run(self):
		results = defaultdict(lambda: [])
		self.write("## Benchmark outputs\n")
		for target in BENCHMARK_TARGETS:
			if ONLY_CPV_FRAMEWORK and target != "cpv-framework":
				continue
			self.write("### %s \n"%target)
			self.write("version: %s"%self.get_version(target))
			self.write()
			self.build(target)
			for case in BENCHMARK_CASES:
				for taskset in BENCHMARK_TASKSETS:
					for command in case["commands"]:
						result = self.benchmark(target, case, taskset, command)
						results[case["category"]].append({
							"target": target,
							"name": "%s / %s"%(taskset["name"], command["name"]),
							"result": result
						})
		self.write("## Charts\n")
		for case in BENCHMARK_CASES:
			self.write("### Chart for %s\n"%case["category"])
			self.write("original data:\n")
			self.write("``` json")
			self.write(json.dumps(results[case["category"]], indent=2))
			self.write("```")
			self.write()

def main():
	runner = BenchmarkRunner()
	runner.run()

if __name__ == "__main__":
	main()

