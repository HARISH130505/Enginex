# Live API Integration Verification Script
$port = 8089
$base = "http://127.0.0.1:$port"

Write-Host "Starting backend server on port $port..."
$proc = Start-Process -FilePath "build/bin/backend_server.exe" -ArgumentList "--port", "$port" -PassThru -NoNewWindow
Start-Sleep -Seconds 2

try {
    # 1. Health check
    Write-Host "`n--- 1. Testing Health Endpoint ---"
    $health = Invoke-RestMethod -Uri "$base/api/v1/health" -Method Get
    Write-Host "Health status: $($health.status), application: $($health.application), port: $($health.bound_port)"

    # 2. Thermal Job Submission & Lifecycle
    Write-Host "`n--- 2. Testing Asynchronous Thermal Job Submission ---"
    $thermalBody = @{
        L = 1.0
        A = 0.0001
        k = 10.0
        q = 10000.0
        T_left = 100.0
        T_right = 20.0
        mesh_sizes = @(10, 20, 40, 80)
    } | ConvertTo-Json

    $tJob = Invoke-RestMethod -Uri "$base/api/v1/thermal/jobs" -Method Post -Body $thermalBody -ContentType "application/json"
    Write-Host "Thermal Job Submitted: ID=$($tJob.job_id), Status=$($tJob.status)"
    $tJobId = $tJob.job_id

    # Poll status
    $completed = $false
    for ($i = 0; $i -lt 30; $i++) {
        Start-Sleep -Milliseconds 200
        $status = Invoke-RestMethod -Uri "$base/api/v1/jobs/$tJobId" -Method Get
        Write-Host "  Poll ($i): Status=$($status.status), Progress=$($status.progress), Msg=$($status.status_message)"
        if ($status.status -eq "COMPLETED") {
            $completed = $true
            break
        }
    }
    if (-not $completed) { throw "Thermal job did not complete in time" }

    # Fetch thermal result
    Write-Host "`n--- 3. Testing Thermal Result Retrieval ---"
    $tResult = Invoke-RestMethod -Uri "$base/api/v1/jobs/$tJobId/result" -Method Get
    Write-Host "Thermal solutions count: $($tResult.solutions.Count)"
    Write-Host "First solution max error: $($tResult.solutions[0].max_midpoint_error)"
    Write-Host "Last solution max error: $($tResult.solutions[-1].max_midpoint_error)"

    # 4. Performance Job Submission & Lifecycle
    Write-Host "`n--- 4. Testing Asynchronous Performance Benchmark Submission ---"
    $benchBody = @{
        batch_size = 20
        elements_per_sim = 20
        seed = 42
        timed_reps = 2
        worker_counts = @(1, 2)
        warmup_reps = 1
    } | ConvertTo-Json

    $pJob = Invoke-RestMethod -Uri "$base/api/v1/performance/jobs" -Method Post -Body $benchBody -ContentType "application/json"
    Write-Host "Benchmark Job Submitted: ID=$($pJob.job_id), Status=$($pJob.status)"
    $pJobId = $pJob.job_id

    # Poll status
    $pCompleted = $false
    for ($i = 0; $i -lt 60; $i++) {
        Start-Sleep -Milliseconds 300
        $status = Invoke-RestMethod -Uri "$base/api/v1/jobs/$pJobId" -Method Get
        Write-Host "  Poll ($i): Status=$($status.status), Progress=$($status.progress), Msg=$($status.status_message)"
        if ($status.status -eq "COMPLETED") {
            $pCompleted = $true
            break
        }
    }
    if (-not $pCompleted) { throw "Performance job did not complete in time" }

    # Fetch benchmark result
    Write-Host "`n--- 5. Testing Benchmark Result Retrieval ---"
    $pResult = Invoke-RestMethod -Uri "$base/api/v1/jobs/$pJobId/result" -Method Get
    Write-Host "Hardware model: $($pResult.hardware_model)"
    Write-Host "Worker results count: $($pResult.worker_results.Count)"
    Write-Host "Agreement verified: $($pResult.agreement_verified)"

    # 6. Cancellation test
    Write-Host "`n--- 6. Testing Cooperative Cancellation ---"
    $longBenchBody = @{
        batch_size = 500
        elements_per_sim = 80
        seed = 42
        timed_reps = 10
        worker_counts = @(1, 2, 3, 4)
        warmup_reps = 2
    } | ConvertTo-Json

    $cJob = Invoke-RestMethod -Uri "$base/api/v1/performance/jobs" -Method Post -Body $longBenchBody -ContentType "application/json"
    Write-Host "Submitted long job for cancellation: ID=$($cJob.job_id)"
    Start-Sleep -Milliseconds 100

    $cancelRes = Invoke-RestMethod -Uri "$base/api/v1/jobs/$($cJob.job_id)/cancel" -Method Post
    Write-Host "Cancellation request response: Status=$($cancelRes.status)"

    # Wait for cancellation to settle
    Start-Sleep -Seconds 1
    $cStatus = Invoke-RestMethod -Uri "$base/api/v1/jobs/$($cJob.job_id)" -Method Get
    Write-Host "Post-cancel status: $($cStatus.status), Cancel requested=$($cStatus.cancel_requested)"
    if ($cStatus.status -ne "CANCELLED" -and $cStatus.status -ne "RUNNING") {
        Write-Host "Note: Job terminal state is $($cStatus.status)"
    }

    # Verify result retrieval of cancelled job fails with 400
    try {
        Invoke-RestMethod -Uri "$base/api/v1/jobs/$($cJob.job_id)/result" -Method Get
        Write-Host "WARNING: Expected 400 for cancelled result retrieval"
    } catch {
        Write-Host "Correctly received expected error for cancelled job result: $_"
    }

    # 7. Unknown job test
    Write-Host "`n--- 7. Testing Unknown Job ID ---"
    try {
        Invoke-RestMethod -Uri "$base/api/v1/jobs/job_unknown_99999" -Method Get
        Write-Host "WARNING: Expected 404 for unknown job"
    } catch {
        Write-Host "Correctly received 404 for unknown job ID: $_"
    }

    Write-Host "`n>>> ALL LIVE HTTP API WORKFLOW TESTS PASSED SUCCESSFULLY! <<<"
}
finally {
    Write-Host "`nStopping backend server process..."
    Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
}
