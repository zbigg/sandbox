/**
 * Goal
 *
 * Test if one can create typemodifier Immutable<T> which rewrites
 * standard type to deeplu immutable version of it.
 *
 * Usage - like Partial, Readonly etc.
 * Example below.
 *
 * Discussion
 *  * shouldn't it be decomposed to Immtuable (shalow) and DeepImmutable ?
 *  * `mutate` is weird i guess
 */

export type Immutable<T> = (
    T extends object ? ImmutableObject<T> :
    T extends Array<infer R> ? ImmutableArray<R> : T
);

export type ImmutableObject<T> = {
    readonly [K in keyof T]: Immutable<T[K]>;
}

export interface ImmutableArray<T> extends ReadonlyArray<Immutable<T>> { }

export function mutate<T, K extends keyof T>(object: T, property: K, value: T[K] | ((x: T[K]) => T[K])): T {
    let r: T = Array.isArray(object) ? object.slice() as any as T : Object.assign({}, object);
    if (typeof value === 'function') {
        r[property] = value(r[property])
    } else {
        r[property] = value;
    }
    return r;
}

export function mutateObject<T>(object: T, mutations: { [K in keyof T]: T[K] | ((x: T[K]) => T[K])} ): T {
    const r: T = Object.assign({}, object);
    for(const key in mutations) {
        const v = mutations[key];
        if (typeof v === 'function') {
            r[key] = v(r[key]);
        } else {
            r[key] = v as any;
        }
    }
    return r;
}

//
// Example
//

interface User {
    name: string;
    config?: { [name: string]: string};
};

const bob: User = {
    name: 'bob'
};

const alice: User = {
    name: 'alice',
    config: { email: 'alice@g.com'}
};

const iBob: Immutable<User> = bob;
// x.name = "a";
//x.config['email'] = 'a';

const newBob = mutate(iBob, 'name', 'Bob');
console.log('newBob', newBob);

const newBob2 = mutateObject(iBob, {
    name: 'NewBob',
    config: (config: typeof iBob.config) => {
        return mutateObject(config, {
            'email': '2'
        })
    }
});
console.log('newBob2', newBob2);
console.log('ibob', iBob);
