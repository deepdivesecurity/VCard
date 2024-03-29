FROM node:8

# Create app directory
WORKDIR /app

# Install app dependencies
COPY package.json /app

RUN npm install
# RUN npm install --only=production

# Bundle app source
COPY . /app

CMD node app.js

EXPOSE 8080
# CMD [ "npm", "start" ]