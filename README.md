# WebAssembly ZXing

With big thanks to Xiao Ling for sorting out the Windows build and WebAssembly build process in [WebAssembly ZXing Windows Build fork](https://github.com/yushulx/zxing-cpp-emscripten) 

## Emscripten Installation
1. Download [emsdk-portable-64bit.zip](https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable-64bit.zip)
2. Fetch the latest registry of available tools:
  
    ```
    emsdk update
    ```
3. Download and install the latest SDK tools:

    ```
    emsdk install latest
    ```
3. Make the "latest" SDK "active" for the current user:

    ```
    emsdk activate latest
    ```
4.  Activate PATH and other environment variables in the current terminal:

    ```
    emsdk_env
    ```

## JavaScript ZXing
To build:

  1. `cd build-js`
  2. Run `configure.bat`
  3. Run `build.bat`
  4. Add the path of **build-js** folder to **IIS**.
  5. Open `http://localhost:2588/test.html`.

To use:

``` javascript
    <script>
		var tick = function () {
			if (window.ZXing) {
				ZXing = ZXing();
				testZXing();
			} else {
				setTimeout(tick, 10);
			}
		};
		tick();

		function testZXing() {
			var img = new Image;
			img.src = 'Qr-10.png';
			img.onload = function () {

				var width = Math.floor(this.width),
					height = Math.floor(this.height);

				var canvas = document.createElement('canvas');
				canvas.style.display = 'block';
				canvas.width = width;
				canvas.height = height;
				var ctx = canvas.getContext('2d');
				// ctx.rotate(Math.random()*0.1-0.05);
				ctx.drawImage(this, 0, 0, width, height);
				var imageData = ctx.getImageData(0, 0, width, height);
				var idd = imageData.data;
				document.body.appendChild(canvas);

				var image = ZXing._resize(width, height);

				for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
					ZXing.HEAPU8[image + j] = idd[i];
				}

				var err = ZXing._decode_qr();
				var res = ZXing._decode_results[0];
				var resultString = '';
				if (res) {
					resultString = String.fromCharCode.apply(null, res[0]);
					ctx.beginPath();
					ctx.moveTo(res[1], res[2]);
					ctx.lineTo(res[3], res[4]);
					ctx.lineTo(res[5], res[6]);
					ctx.lineTo(res[7], res[8]);
					ctx.closePath();
					ctx.lineWidth = 4;
					ctx.strokeStyle = 'red';
					ctx.stroke();
					console.log("result", resultString);
				}

				document.body.appendChild(document.createTextNode(err ? ("error: " + err) : resultString));

				for (var k = 0; k < 50; k++) {
					for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
						ZXing.HEAPU8[image + j] = idd[i];
					}
					err = ZXing._decode_qr_multi();
					err = ZXing._decode_qr();
				}

				console.time("decode QR");
				for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
					ZXing.HEAPU8[image + j] = idd[i];
				}
				err = ZXing._decode_qr();
				console.timeEnd("decode QR");

				console.time("decode QR multi");
				for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
					ZXing.HEAPU8[image + j] = idd[i];
				}
				err = ZXing._decode_qr_multi();
				console.timeEnd("decode QR multi");

				console.time("decode any");
				for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
					ZXing.HEAPU8[image + j] = idd[i];
				}
				err = ZXing._decode_any();
				console.timeEnd("decode any");

				console.time("decode multi");
				for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
					ZXing.HEAPU8[image + j] = idd[i];
				}
				err = ZXing._decode_multi();
				console.timeEnd("decode multi");

			};
		};
	</script>
  <script async src="zxing.js"></script>
```

## WebAssembly ZXing
Windows build:

  1. `cd build-wasm`
  2. Run `configure.bat`
  3. Run `build.bat`
  4. Add the path of **build-wasm** folder to **IIS**.
  5. Open `http://localhost:2588/test.html`.

Linux / OSX build:

  1. `cd build-wasm`
  2. `cp CMakeLists.txt ..`
	3. `emconfigure cmake ..`
	4. `emmake make -j8`
	5. `serve`
	6. Open `http://localhost:5000/test.html`

To use:

``` javascript
    <script>
		var ZXing;
		var Module = {
			onRuntimeInitialized: function () {
				ZXing = Module;
				testZXing();
			}
		};

		function testZXing() {
			var img = new Image;
			img.src = 'Qr-10.png';
			img.onload = function () {

				var width = Math.floor(this.width),
					height = Math.floor(this.height);

				var canvas = document.createElement('canvas');
				canvas.style.display = 'block';
				canvas.width = width;
				canvas.height = height;
				var ctx = canvas.getContext('2d');
				// ctx.rotate(Math.random()*0.1-0.05);
				ctx.drawImage(this, 0, 0, width, height);
				var imageData = ctx.getImageData(0, 0, width, height);
				var idd = imageData.data;
				document.body.appendChild(canvas);

				var image = ZXing._resize(width, height);

				for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
					ZXing.HEAPU8[image + j] = idd[i];
				}

				var err = ZXing._decode_qr();
				var res = ZXing._decode_results[0];
				var resultString = '';
				if (res) {
					resultString = String.fromCharCode.apply(null, res[0]);
					ctx.beginPath();
					ctx.moveTo(res[1], res[2]);
					ctx.lineTo(res[3], res[4]);
					ctx.lineTo(res[5], res[6]);
					ctx.lineTo(res[7], res[8]);
					ctx.closePath();
					ctx.lineWidth = 4;
					ctx.strokeStyle = 'red';
					ctx.stroke();
					console.log("result", resultString);
				}

				document.body.appendChild(document.createTextNode(err ? ("error: " + err) : resultString));

				for (var k = 0; k < 50; k++) {
					for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
						ZXing.HEAPU8[image + j] = idd[i];
					}
					err = ZXing._decode_qr_multi();
					err = ZXing._decode_qr();
				}

				console.time("decode QR");
				for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
					ZXing.HEAPU8[image + j] = idd[i];
				}
				err = ZXing._decode_qr();
				console.timeEnd("decode QR");

				console.time("decode QR multi");
				for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
					ZXing.HEAPU8[image + j] = idd[i];
				}
				err = ZXing._decode_qr_multi();
				console.timeEnd("decode QR multi");

				console.time("decode any");
				for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
					ZXing.HEAPU8[image + j] = idd[i];
				}
				err = ZXing._decode_any();
				console.timeEnd("decode any");

				console.time("decode multi");
				for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
					ZXing.HEAPU8[image + j] = idd[i];
				}
				err = ZXing._decode_multi();
				console.timeEnd("decode multi");

			};
		};
	</script>
  <script async src="zxing.js"></script>
```

## Performance

![JavaScript ZXing vs WebAssembly ZXing](http://www.codepool.biz/wp-content/uploads/2018/01/javascript-webassembly-zxing-performance.PNG)

## Blog
- [Building JavaScript and WebAssembly ZXing on Windows](http://www.codepool.biz/building-webassembly-zxing-windows.html)
- [Building HTML5 Barcode Reader with Pure JavaScript SDK](http://www.codepool.biz/html5-barcode-reader-javascript-webassembly.html)
