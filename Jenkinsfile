pipeline {
    agent any
    
    environment {
        // 基础配置
        PROJECT_NAME = "frs"
        GITHUB_NAMESPACE = "yangxiangmin"
        DOCKER_REGISTRY = "dockhub.ghtchina.com:6060"
        DOCKER_NAMESPACE = "ims-cloud"
        
        // 动态计算的变量
        SOURCE_CODE_URL = "https://github.com/${GITHUB_NAMESPACE}/${PROJECT_NAME}.git"
        APP_IMAGE_URL = "${DOCKER_REGISTRY}/${DOCKER_NAMESPACE}/${PROJECT_NAME}"
        BASE_IMAGE = ""
        BUILD_DIR = "${WORKSPACE}/${PROJECT_NAME}"
    }
    
    stages {
        stage('初始化') {
            steps {
                script {
                    echo "工作空间路径: ${env.WORKSPACE}"
                    echo "初始化项目: ${PROJECT_NAME}"
                    echo "源代码URL: ${SOURCE_CODE_URL}"
                    echo "应用镜像URL: ${APP_IMAGE_URL}"
                }
            }
        }
        
        stage('获取代码') {
            steps {
                script {
                    try {
                        echo "从 ${SOURCE_CODE_URL} 克隆代码..."
                        git url: SOURCE_CODE_URL, branch: 'main', credentialsId: 'github-credentials'
                        echo "代码克隆成功"
                    } catch (Exception e) {
                        error "克隆代码失败: ${e.getMessage()}"
                    }
                }
            }
        }
        
        stage('解析Dockerfile') {
            steps {
                script {
                    try {
                        //def dockerfilePath = "${WORKSPACE}/${PROJECT_NAME}/dockfile/Dockerfile"
                        def dockerfilePath = "${WORKSPACE}/dockfile/Dockerfile"
                        echo "解析Dockerfile: ${dockerfilePath}"

                        // 详细检查文件是否存在
                        if (!fileExists(dockerfilePath)) {
                            // 打印目录内容帮助调试
                            sh "ls -la ${WORKSPACE}/dockfile/ || true"
                            error "Dockerfile 不存在于路径: ${dockerfilePath}\n当前目录内容如上所示"
                        }

                        // 读取Dockerfile内容
                        def dockerfileContent = readFile(dockerfilePath)
                        
                        // 提取FROM指令
                        def fromLine = dockerfileContent.readLines().find { it.startsWith('FROM ') }
                        if (!fromLine) {
                            error "Dockerfile中没有找到FROM指令"
                        }
                        
                        // 获取基础镜像
                        BASE_IMAGE = fromLine.substring(5).trim()
                        echo "基础镜像: ${BASE_IMAGE}"
                        
                        // 保存到环境变量供后续阶段使用
                        env.BASE_IMAGE = BASE_IMAGE
                    } catch (Exception e) {
                        error "解析Dockerfile失败: ${e.getMessage()}"
                    }
                }
            }
        }

        stage('拉取基础镜像') {
            steps {
                script {
                    try {
                        echo "拉取基础镜像: ${BASE_IMAGE}"
                        sh "docker pull ${BASE_IMAGE}"
                        // 简单验证镜像可用性
                        sh "docker inspect ${BASE_IMAGE} >/dev/null"
                        echo "基础镜像拉取成功"
                    } catch (Exception e) {
                        error "拉取基础镜像失败: ${e.getMessage()}"
                    }
                }
            }
        }

        stage('编译和测试') {
            steps {
                script {
                    try {
                        def buildDependencies = "gcc gcc-c++ make cmake3 jsoncpp-devel openssl-devel git"
                        
                        echo "在基础镜像中执行编译和测试..."
                        
                        // 使用 returnStatus 获取命令退出状态
                        def compileResult = sh(
                            script: """
                            #!/bin/bash
                            set -xe
                            
                            # 使用docker run直接运行容器执行编译
                            # 如果编译输出不在src目录下，则还需要增加相关目录情况挂载，yangxmflag
                            docker run --rm \
                                -v ${WORKSPACE}/src:/workspace/src \
                                -v ${WORKSPACE}/.cache:/root/.cache \
                                -w /workspace/src \
                                ${BASE_IMAGE} /bin/bash -c '

                                    # 打印初始工作目录
                                    echo "当前工作目录: \$(pwd)"

                                    # 1. 可靠的系统检测和依赖安装
                                    install_deps() {
                                        # 检测包管理器
                                        if command -v yum >/dev/null; then
                                            echo "安装RHEL/CentOS依赖..."
                                            yum install -y epel-release || \\
                                                yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
                                            yum install -y \$1
                                            [[ -f /usr/bin/cmake3 ]] && ln -sf /usr/bin/cmake3 /usr/bin/cmake
                                        elif command -v apt-get >/dev/null; then
                                            echo "安装Debian/Ubuntu依赖..."
                                            apt-get update -qq
                                            apt-get install -y \$2
                                        else
                                            echo "无法识别的Linux发行版"
                                            exit 1
                                        fi
                                    }
                                    
                                    # 安装依赖（参数：RHEL依赖包 Debian依赖包）
                                    install_deps "${buildDependencies}" "build-essential cmake libjsoncpp-dev libssl-dev"
                                    
                                    # 2. 验证环境
                                    echo "===== 环境验证 ====="
                                    g++ --version || { echo "[ERROR] g++未安装"; exit 1; }
                                    cmake --version || cmake3 --version || { echo "[ERROR] cmake未安装"; exit 1; }

                                    echo "===== 创建软连接 ====="
                                    cd lib64
                                    ln -sf libaws-c-auth.so.1.0.0 libaws-c-auth.so
                                    ln -sf libaws-c-cal.so.1.0.0 libaws-c-cal.so
                                    ln -sf libaws-c-common.so.1.0.0 libaws-c-common.so.1
                                    ln -sf libaws-c-common.so.1 libaws-c-common.so
                                    ln -sf libaws-c-compression.so.1.0.0 libaws-c-compression.so
                                    ln -sf libaws-c-event-stream.so.1.0.0 libaws-c-event-stream.so
                                    ln -sf libaws-checksums.so.1.0.0 libaws-checksums.so
                                    ln -sf libaws-c-http.so.1.0.0 libaws-c-http.so
                                    ln -sf libaws-c-io.so.1.0.0 libaws-c-io.so
                                    ln -sf libaws-c-mqtt.so.1.0.0 libaws-c-mqtt.so
                                    ln -sf libaws-c-s3.so.1.0.0 libaws-c-s3.so.0unstable
                                    ln -sf libaws-c-s3.so.0unstable libaws-c-s3.so
                                    ln -sf libaws-c-sdkutils.so.1.0.0 libaws-c-sdkutils.so
                                    ln -sf libs2n.so.1.0.0 libs2n.so.1
                                    ln -sf libs2n.so.1 libs2n.so
                                    cd ..

                                    # 打印编译前工作目录
                                    echo "编译前工作目录: \$(pwd)"

                                    # 3. 编译和测试（将错误输出到文件）
                                    echo "===== 开始编译 ====="
                                    make clean
                                    
                                    # 编译并将错误输出到文件
                                    make -j\$(nproc) 2> build_errors.log || {
                                        echo "[BUILD FAILED] 编译错误："
                                        cat build_errors.log
                                        exit 1
                                    }
                                    
                                    if grep -q "^test:" Makefile; then
                                        echo "===== 运行测试 ====="
                                        make test 2> test_errors.log || {
                                            echo "[TEST FAILED] 测试失败："
                                            cat test_errors.log
                                            exit 1
                                        }
                                    fi
                                '
                            """,
                            returnStatus: true  // 获取命令返回状态码
                        )
                        
                        // 检查编译结果
                        if (compileResult != 0) {
                            // 获取容器日志（如果需要）
                            def containerLog = sh(
                                script: "docker ps -lq | xargs docker logs 2>&1 | tail -n 50 || true",
                                returnStdout: true
                            )
                            error "编译测试失败，返回码: ${compileResult}\n容器日志片段:\n${containerLog}"
                        }
                        
                        echo "容器内编译测试完成"
                    } catch (Exception e) {
                        // 捕获异常并打印详细错误
                        def errorLog = sh(
                            script: "cat ${WORKSPACE}/src/build_errors.log ${WORKSPACE}/src/test_errors.log 2>/dev/null || true",
                            returnStdout: true
                        )
                        error "编译测试阶段失败:\n${e.getMessage()}\n错误日志:\n${errorLog}"
                    }
                }
            }
        }

        stage('构建应用镜像') {
            steps {
                script {
                    try {
                        // 获取当前时间戳或Git提交ID作为标签
                        // def tag = sh(script: "git rev-parse --short HEAD", returnStdout: true).trim()

                        // 直接使用预定义的 BUILD_NUMBER 环境变量
                        def tag = env.BUILD_NUMBER
                        env.IMAGE_TAG = tag
                        
                        echo "构建应用镜像: ${APP_IMAGE_URL}:${tag}"
                        
                        //dir("${WORKSPACE}/${PROJECT_NAME}/dockfile") {
                        dir("${WORKSPACE}/dockfile") {

                            // 将编译产物考到dockerfile，要根据实际情况，包括make编译输出目录以及Dockerfile里面的具体定义
                            cp ${WORKSPACE}/src/frssvr .
                            cp -r ${WORKSPACE}/src/lib64 .

                            // 构建Docker镜像
                            sh "docker build -t ${APP_IMAGE_URL}:${tag} ."
                        }
                        
                        echo "应用镜像构建成功"
                    } catch (Exception e) {
                        error "构建应用镜像失败: ${e.getMessage()}"
                    }
                }
            }
        }
        
        stage('推送镜像') {
            steps {
                script {
                    try {
                        echo "登录Docker仓库..."
                        withCredentials([usernamePassword(credentialsId: 'dockerhub-credentials', 
                                        usernameVariable: 'DOCKER_USER', 
                                        passwordVariable: 'DOCKER_PASS')]) {
                            sh "echo ${DOCKER_PASS} | docker login -u ${DOCKER_USER} --password-stdin ${DOCKER_REGISTRY}"
                        }
                        
                        echo "推送镜像 ${APP_IMAGE_URL}:${IMAGE_TAG} 到仓库..."
                        sh "docker push ${APP_IMAGE_URL}:${IMAGE_TAG}"
                        
                        // 同时标记为latest并推送
                        sh "docker tag ${APP_IMAGE_URL}:${IMAGE_TAG} ${APP_IMAGE_URL}:latest"
                        sh "docker push ${APP_IMAGE_URL}:latest"
                        
                        echo "镜像推送成功"
                    } catch (Exception e) {
                        error "推送镜像失败: ${e.getMessage()}"
                    } finally {
                        // 登出Docker仓库
                        sh "docker logout ${DOCKER_REGISTRY}"
                    }
                }
            }
        }
        
        stage('Kubernetes部署') {
            steps {
                script {
                    try {
                        echo "准备Kubernetes部署..."
                        //dir("${WORKSPACE}/${PROJECT_NAME}/k8s-yaml") {
                        dir("${WORKSPACE}/k8s-yaml") {
                            // 使用sed替换yaml文件中的镜像标签
                            sh """
                                sed -i 's|image: ${APP_IMAGE_URL}:.*|image: ${APP_IMAGE_URL}:${IMAGE_TAG}|g' *.yaml
                            """
                            
                            // 应用k8s配置 (假设有kubectl配置)
                            sh "kubectl apply -f ."
                        }
                        echo "Kubernetes部署完成"
                    } catch (Exception e) {
                        error "Kubernetes部署失败: ${e.getMessage()}"
                    }
                }
            }
        }
    }
    
    post {
        always {
            echo "清理工作空间..."
            sh "rm -f ${WORKSPACE}/src/*.log || true"
            cleanWs()
        }
        success {
            echo "流水线执行成功"
            // 可以添加通知逻辑
        }
        failure {
            echo "流水线执行失败"
            // 可以添加失败通知逻辑
        }
    }
}