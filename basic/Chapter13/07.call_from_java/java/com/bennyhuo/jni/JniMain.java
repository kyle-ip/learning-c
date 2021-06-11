package com.bennyhuo.jni;

public class JniMain {

    static {
        System.loadLibrary("07.call_from_java");
    }

    public static native int fibonacci(int num);

    public static void main(String[] args) {
        System.out.println(fibonacci(5));
    }
}