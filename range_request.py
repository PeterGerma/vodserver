import requests

url = "http://localhost:8000/file.txt"
headers = {"Range": "bytes=0-100"}
r = requests.get(url,headers=headers)#
assert len(r.text) <= 101

print(len(r.text))
print(r.status_code)
print(r.headers)
print(r.text)