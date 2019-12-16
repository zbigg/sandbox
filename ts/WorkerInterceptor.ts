import {baseUrl, resolveReferenceUrl} from "@here/harp-utils";

function fakeWebWorkerBootstrap() {
    interface WorkerGlobalScope {
        addEventListener(name: string, listener: (event: MessageEvent) => void): void;
        postMessage(message: any): void;
        importScripts(foo: string): void;
    }
    const workerSelf = (self as any) as WorkerGlobalScope;

    console.log("FWWB #init");
    workerSelf.postMessage({ $$type: "init" });

    workerSelf.addEventListener("message", (messageEvent: MessageEvent) => {
        const message = messageEvent.data;
        console.log("FWWB #om", messageEvent);
        if (message.$$type === "init") {
            const script = message.scripts;
            const originalWorkerScripts = workerSelf.importScripts;

            workerSelf.importScripts = (subScript: string) => {
                const relUrl = new URL(subScript, script).href;
                console.log("FFWB #fis", relUrl);
                originalWorkerScripts(relUrl);
            }
            originalWorkerScripts(script);
        }
        if (message.$$type === "gatherStats") {
            workerSelf.postMessage({ $$type: "stats", entries: performance.getEntries() });
        }
    });
}

class FakeWorker implements Worker {
    static originalWorkerConstructor: new (str: string) => Worker;
    targetWorker: Worker;
    resolve!: (worker: Worker) => void;
    workerReady: Promise<Worker>;

    constructor(readonly originalWorkerScript: string) {
        const fakeWorkerCode = `${baseUrl};${fakeWebWorkerBootstrap};\nfakeWebWorkerBootstrap();`;
        const blob = new Blob([fakeWorkerCode], { type: "application/javascript" });
        const blobURL = URL.createObjectURL(blob);
        console.log("FW const", resolveReferenceUrl(window.location.href, originalWorkerScript));
        this.targetWorker = new FakeWorker.originalWorkerConstructor(blobURL);

        this.targetWorker.addEventListener("error", this.onError);
        this.targetWorker.addEventListener("message", this.onMessage);

        this.workerReady = new Promise(resolve => {
            this.resolve = resolve;
        });
    }

    onError = (error: ErrorEvent) => {
        // tslint:disable-next-line:no-console
        console.error("FakeWorker#onError", error);
    };

    onMessage = (messageEvent: MessageEvent) => {
        const message = messageEvent.data;
        console.log("FW #om", message);
        if (message.$$type === "init") {
            const url = resolveReferenceUrl(window.location.href, this.originalWorkerScript);
            this.targetWorker.postMessage({
                $$type: "init",
                scripts: url,
                baseUrl: baseUrl(url)
            });
            this.resolve(this.targetWorker);
        } else if (message.$$type === "stats") {
            // tslint:disable-next-line:no-console
            console.log("STATS from worker", message.entries);
        }
    };

    set onmessage(foo: any) {
        // wait ?
        this.targetWorker.onmessage = foo;
    }
    set onerror(foo: any) {
        this.targetWorker.onerror = foo;
    }

    postMessage(message: any, transfer: Transferable[]): void;
    postMessage(message: any, options?: PostMessageOptions | undefined): void;
    postMessage(message: any, options?: any) {
        this.workerReady.then(worker => {
            worker.postMessage(message, options);
        });
    }
    terminate(): void {
        this.workerReady.then(worker => {
            worker.terminate();
        });
    }
    addEventListener<K extends "message" | "error">(
        type: K,
        listener: (this: Worker, ev: WorkerEventMap[K]) => void,
        options?: boolean | AddEventListenerOptions | undefined
    ): void;
    addEventListener(
        type: string,
        listener: EventListenerOrEventListenerObject,
        options?: boolean | AddEventListenerOptions | undefined
    ): void;
    addEventListener(type: any, listener: any, options?: any) {
        this.workerReady.then(worker => {
            worker.addEventListener(type, listener, options);
        });
    }
    removeEventListener<K extends "message" | "error">(
        type: K,
        listener: (this: Worker, ev: WorkerEventMap[K]) => void,
        options?: boolean | EventListenerOptions | undefined
    ): void;
    removeEventListener(
        type: string,
        listener: EventListenerOrEventListenerObject,
        options?: boolean | EventListenerOptions | undefined
    ): void;
    removeEventListener(type: any, listener: any, options?: any) {
        this.workerReady.then(worker => {
            worker.removeEventListener(type, listener, options);
        });
    }
    dispatchEvent(event: Event): boolean {
        // NOT sure if this is valid!
        if (!this.targetWorker) {
            return false;
        }
        return this.targetWorker.dispatchEvent(event);
    }
}

FakeWorker.originalWorkerConstructor = Worker;
(window as any).Worker = FakeWorker;
