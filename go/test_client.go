package main

import (
	"crypto/rand"
	"flag"
	"fmt"
	"log"
	"net"
	"sync"
	"sync/atomic"
	"time"
)

// 生成指定长度的随机字节切片
func generateRandomBytes(n int) []byte {
	b := make([]byte, n)
	_, err := rand.Read(b)
	if err != nil {
		log.Fatal(err)
	}
	return b
}

// 模拟单个客户端的行为
func simulateClient(targetAddr string, payloadSize int, wg *sync.WaitGroup, successCount *int32, totalLatency *int64) {
	defer wg.Done()

	start := time.Now()

	// 1. 建立 TCP 连接
	conn, err := net.DialTimeout("tcp", targetAddr, 3*time.Second)
	if err != nil {
		// 连接失败不阻塞主程序，直接返回
		return
	}
	defer conn.Close()

	// 2. 发送随机数据
	payload := generateRandomBytes(payloadSize)
	_, err = conn.Write(payload)
	if err != nil {
		return
	}

	// 3. 接收 Echo 回复
	buf := make([]byte, payloadSize)
	// 读取服务器返回的数据
	_, err = conn.Read(buf)
	if err != nil {
		return
	}

	// 计算耗时
	latency := time.Since(start).Milliseconds()

	// 原子操作更新统计数据，避免并发冲突
	atomic.AddInt32(successCount, 1)
	atomic.AddInt64(totalLatency, latency)
}

func main() {
	// 定义命令行参数
	concurrency := flag.Int("c", 100, "并发数量 (例如: 10, 100, 1000)")
	payloadSize := flag.Int("s", 512, "发送随机数据的长度 (10~1000)")
	address := flag.String("addr", "127.0.0.1:8080", "服务器地址")
	flag.Parse()

	if *payloadSize < 10 || *payloadSize > 1000 {
		log.Fatal("内容长度必须在 10 到 1000 之间")
	}

	fmt.Printf("开始压测...\n")
	fmt.Printf("目标: %s | 并发数: %d | 数据包大小: %d bytes\n", *address, *concurrency, *payloadSize)

	var wg sync.WaitGroup
	var successCount int32 = 0
	var totalLatency int64 = 0

	startTime := time.Now()

	// 发起并发请求
	for i := 0; i < *concurrency; i++ {
		wg.Add(1)
		go simulateClient(*address, *payloadSize, &wg, &successCount, &totalLatency)
	}

	// 等待所有 Goroutine 执行完毕
	wg.Wait()

	totalTime := time.Since(startTime)

	// 打印统计报告
	fmt.Printf("\n====== 压测报告 ======\n")
	fmt.Printf("总请求数:     %d\n", *concurrency)
	fmt.Printf("成功请求数:   %d\n", successCount)
	fmt.Printf("失败请求数:   %d\n", int32(*concurrency)-successCount)
	fmt.Printf("总耗时:       %v\n", totalTime)

	if successCount > 0 {
		avgLatency := float64(totalLatency) / float64(successCount)
		fmt.Printf("平均请求延迟: %.2f ms\n", avgLatency)
		// QPS (Queries Per Second) = 成功数 / 总耗时(秒)
		qps := float64(successCount) / totalTime.Seconds()
		fmt.Printf("QPS (吞吐量): %.2f req/sec\n", qps)
	}
	fmt.Printf("======================\n")
}
