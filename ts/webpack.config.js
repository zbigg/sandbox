const path = require('path');

module.exports = {
  entry: './hello.ts',
  target: 'node',
  output: {
    path: __dirname,
    filename: 'hello-bundle.js'
  },
  resolve: {
    // Add `.ts` and `.tsx` as a resolvable extension.
    extensions: ['.ts', '.tsx', '.js']
  },
  module: {
    loaders: [
      // all files with a `.ts` or `.tsx` extension will be handled by `ts-loader`
      {
        test: /\.tsx?$/,
        loader: 'ts-loader'
      }
    ]
  }
}
