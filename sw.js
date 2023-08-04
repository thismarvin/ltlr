// The following is yoinked from:
// https://developer.mozilla.org/en-US/docs/Web/Progressive_web_apps/Offline_Service_workers

const cacheName = "ltlr-2023-08-04";
const contentToCache = [
	"FileSaver.min.js",
	"android-chrome-192x192.png",
	"android-chrome-512x512.png",
	"apple-touch-icon-precomposed.png",
	"apple-touch-icon.png",
	"favicon-16x16.png",
	"favicon-32x32.png",
	"favicon-48x48.png",
	"favicon.ico",
	"index.data",
	"index.html",
	"index.js",
	"index.wasm",
	"maskable-196x196.png",
	"site.webmanifest",
	"sw.js",
];

self.addEventListener("install", (e) => {
	console.log("[Service Worker] Install");

	e.waitUntil(
		(async () => {
			const cache = await caches.open(cacheName);

			console.log("[Service Worker] Caching content");

			await cache.addAll(contentToCache);
		})()
	);
});

self.addEventListener("fetch", (e) => {
	e.respondWith(
		(async () => {
			const r = await caches.match(e.request);

			console.log(`[Service Worker] Fetching resource: ${e.request.url}`);

			if (r !== undefined) {
				return r;
			}

			const response = await fetch(e.request);
			const cache = await caches.open(cacheName);

			console.log(`[Service Worker] Caching new resource: ${e.request.url}`);

			cache.put(e.request, response.clone());

			return response;
		})()
	);
});

self.addEventListener("activate", (e) => {
	e.waitUntil(
		caches.keys().then((keyList) => {
			return Promise.all(
				keyList.map((key) => {
					if (key === cacheName) {
						return;
					}

					return caches.delete(key);
				})
			);
		})
	);
});
