#coding:utf-8

import json
import os
import sys
import hashlib
import shutil

# 打开并加载配置文件
configFile = open("profileConfig.json")
profileConfig = json.loads(configFile.read())
configFile.close()

if os.path.exists("news"):
	print "directory news already exists, backup and delete it first..."
	sys.exit()
else:
	os.mkdir("news")
	

# 循环每个model的配置，计算md5，复制文件和修改配置
for modelConfig in profileConfig:
	fileName = modelConfig["fileName"]
	originalMd5 = modelConfig["md5"]
	if os.path.exists(fileName):
		# 计算配置文件的md5
		profileFile = open(fileName,"r")
		md5 = hashlib.md5(profileFile.read()).hexdigest()
		profileFile.close()
		# 如果不一致则复制文件、修改名称并且修改json配置
		if(originalMd5 != md5):
			print fileName," changed, md5 from ",originalMd5," to ",md5
			shutil.copy(fileName, "news/" + md5)
			modelConfig["md5"] = md5
		else:
			print fileName," has no change"

newConfigFile = open("news/profileConfig.json", "w")
newConfigFile.write(json.dumps(profileConfig))
newConfigFile.close()