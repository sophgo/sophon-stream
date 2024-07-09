安装node.js
```bash
# installs nvm (Node Version Manager)
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
# download and install Node.js (you may need to restart the terminal)
nvm install 20
# verifies the right Node.js version is in the environment
node -v # should print `v20.15.1`
# verifies the right NPM version is in the environment
npm -v # should print `10.7.0`
```

安装依赖
```bash
npm install
```

启动服务
```bash
npm run dev
```