# IVS engine开发流程

## 建立开发环境

* 首先进入代码服务器上的ivs engine首页
  http://192.168.8.18:3000/IVS/ivs-engine
  点击右上角的"派生"。
* 派生完后进入自己的私有仓库，如：
  http://192.168.8.18:3000/xin.ouyang/ivs-engine
* 进入"设置"里，启用"合并请求"
* 使用git clone命令

    ```
    git clone http://192.168.8.18:3000/xin.ouyang/ivs-engine
    ```

* 进入clone下来的目录，添加远程仓库

    ```
    cd ivs-engine
    git remote add upstream http://192.168.8.18:3000/IVS/ivs-engine
    ```

## 日常同步代码

    git pull --rebase upstream master

## 提交和推送代码

    git push (origin engine)

   推送完代码后，进入自己的私有仓库，如：
   http://192.168.8.18:3000/xin.ouyang/ivs-engine
   发起合并请求pull request。