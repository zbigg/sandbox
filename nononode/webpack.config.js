const webpack = require('webpack');
const path = require('path');

module.exports = {
    devtool: 'source-map',
    resolve: {
        extensions: ['.js', '.ts', '.jsx', 'json']
    },
    entry: ['./index.js'],
    output: {
        path: path.join(__dirname, 'dist'),
        filename: 'foo.js'
    },
    module: {
        rules: [
            {
                test: /\jsx?$/,
                loader: 'babel-loader',
                exclude: /node_modules/
            },
        ]
    }
};