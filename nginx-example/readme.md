```shell
docker build -t nginx-example .
docker run --name nginx-example --volume=YOUR_STATIC_PATH:/static -d -p 80:80 nginx-example
docker stop nginx-example
docker rm nginx-example
```
http://localhost/static/

