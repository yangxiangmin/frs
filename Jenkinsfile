pipeline {
    agent any
    
    environment {
        // 基础配置
        //PROJECT_NAME = "frs"
        PROJECT_NAME = env.JOB_NAME.split('/').last()                                   // 将任务名按 / 分割，取最后一部分作为项目名
        GITHUB_NAMESPACE = "yangxiangmin"
        DOCKER_REGISTRY = "dockhub.ghtchina.com:6060"
        DOCKER_NAMESPACE = "ims-cloud"
        SOURCE_CODE_URL_SVN = "https://svn.harris.guangzhou.gd.cn/svn/IMS/IMSas/econfig" // SVN源代码URL通过静态设置设置
        
        // 动态计算的变量
        SOURCE_CODE_URL = "https://github.com/${GITHUB_NAMESPACE}/${PROJECT_NAME}.git"  // github源代码URL通过动态方式设置
        APP_IMAGE_URL = "${DOCKER_REGISTRY}/${DOCKER_NAMESPACE}/${PROJECT_NAME}"
        BASE_IMAGE = ""
        BUILD_DIR = "${WORKSPACE}/${PROJECT_NAME}"

        BUILD_DEPENDENCIES = "gcc gcc-c++ make cmake3 jsoncpp-devel openssl-devel git"  // 自定义RHEL/CentOS依赖，根据实际基础镜像和应用编译环境作相应修改
        DEBIAN_DEPENDENCIES = "build-essential cmake libjsoncpp-dev libssl-dev"         // 自定义Debian/Ubuntu依赖，根据实际基础镜像和应用编译环境作相应修改
        BUILD_OUT_DIR = "buildout"                                                      // 自定义编译输出目录名称，根据Makefile实际编译输出目录作相应修改
        BUILD_ERR_DIR = "builderrlog"                                                   // 编译错误日志目录
    }
    
    stages {
        stage('初始化') {
            steps {
                script {
                    echo "当前处理节点: ${env.NODE_NAME}"
                    echo "工作空间路径: ${env.WORKSPACE}"
                    echo "初始化项目: ${PROJECT_NAME}"
                    echo "源代码URL: ${SOURCE_CODE_URL}"
                    echo "应用镜像URL: ${APP_IMAGE_URL}"
                    echo "应用编译输出目录: ${BUILD_OUT_DIR}"

                }
            }
        }

        stage('svn方式获取代码') {
            steps {
                script {
                    // 弹出输入框获取凭据
                    def svnCredentials = input(
                        id: 'svnAuth',
                        message: '请输入SVN账号密码',
                        parameters: [
                            string(name: 'username', description: 'SVN账号'),
                            password(name: 'password', description: 'SVN密码')
                        ]
                    )

                    checkout([
                        $class: 'SubversionSCM',
                        locations: [[
                            remote: "${env.SOURCE_CODE_URL_SVN}",
                            credentialsId: '',
                            username: "${svnCredentials.username}",
                            password: "${svnCredentials.password}",
                            depthOption: 'infinity',
                            ignoreExternalsOption: true
                        ]],
                        workspaceUpdater: [$class: 'UpdateWithCleanUpdater']
                    ])
                }
            }
        }
/*
        stage('svn方式获取代码') {
            steps {
                checkout([
                    $class: 'SubversionSCM',
                    locations: [[
                        // 使用双引号确保变量解析
                        remote: "${env.SOURCE_CODE_URL_SVN}",
                        // 直接使用凭证ID（需在Jenkins后台配置）
                        credentialsId: 'svn-credential',
                        depthOption: 'infinity',
                        ignoreExternalsOption: true
                    ]],
                    workspaceUpdater: [$class: 'UpdateWithCleanUpdater']
                ])
            }
        }


        stage('git方式获取代码') {
            steps {
                script {
                    echo "当前处理节点: ${env.NODE_NAME}"
                    try {
                        echo "从 ${SOURCE_CODE_URL} 克隆代码..."
                        git url: SOURCE_CODE_URL, branch: 'main', credentialsId: 'github-credentials'
                        echo "✅ 代码克隆成功"
                    } catch (Exception e) {
                        error "❌ 代码克隆失败: ${e.getMessage()}"
                    }
                }
            }
        }
*/        
        stage('解析Dockerfile') {
            steps {
                echo "当前处理节点: ${env.NODE_NAME}"
                script {
                    try {
                        //def dockerfilePath = "${WORKSPACE}/${PROJECT_NAME}/dockfile/Dockerfile"
                        def dockerfilePath = "${WORKSPACE}/dockfile/Dockerfile"
                        echo "解析Dockerfile: ${dockerfilePath}"

                        // 详细检查文件是否存在
                        if (!fileExists(dockerfilePath)) {
                            // 打印目录内容帮助调试
                            sh "ls -la ${WORKSPACE}/dockfile/ || true"
                            error "❌ Dockerfile 不存在于路径: ${dockerfilePath}\n当前目录内容如上所示"
                        }

                        // 读取Dockerfile内容
                        def dockerfileContent = readFile(dockerfilePath)
                        
                        // 提取FROM指令
                        def fromLine = dockerfileContent.readLines().find { it.startsWith('FROM ') }
                        if (!fromLine) {
                            error "❌ Dockerfile中没有找到FROM指令"
                        }
                        
                        // 获取基础镜像
                        BASE_IMAGE = fromLine.substring(5).trim()
                        echo "✅ 获取基础镜像要求成功: ${BASE_IMAGE}"
                        
                        // 保存到环境变量供后续阶段使用
                        env.BASE_IMAGE = BASE_IMAGE
                    } catch (Exception e) {
                        error "❌ Dockerfile解析失败: ${e.getMessage()}"
                    }
                }
            }
        }

        stage('拉取基础镜像') {
            steps {
                echo "当前处理节点: ${env.NODE_NAME}"
                script {
                    try {
                        echo "拉取基础镜像: ${BASE_IMAGE}"
                        sh "docker pull ${BASE_IMAGE}"
                        // 简单验证镜像可用性
                        sh "docker inspect ${BASE_IMAGE} >/dev/null"
                        echo "✅ 基础镜像拉取成功"
                    } catch (Exception e) {
                        error "❌ 基础镜像拉取失败: ${e.getMessage()}"
                    }
                }
            }
        }


        stage('编译和测试') {
            steps {
                echo "当前处理节点: ${env.NODE_NAME}"
                script {
                    try {
                        echo "在基础镜像中执行编译和测试..."
                        
                        def compileResult = sh(
                            script: """
                            #!/bin/bash
                            set -xe
                            
                            # 挂载整个工作空间到容器内
                            docker run --rm \
                                -v ${WORKSPACE}:/workspace \
                                -v ${WORKSPACE}/.cache:/root/.cache \
                                -w /workspace \
                                ${BASE_IMAGE} /bin/bash -c '
                                    # 打印初始工作目录
                                    echo "当前工作目录: \$(pwd)"

                                    # 1. 可靠的系统检测和依赖安装
                                    install_deps() {
                                        # 检测包管理器
                                        if command -v yum >/dev/null; then
                                            echo "安装RHEL/CentOS依赖..."
                                            yum install -y ${env.BUILD_DEPENDENCIES} || exit 1
                                        elif command -v apt-get >/dev/null; then
                                            echo "安装Debian/Ubuntu依赖..."
                                            apt-get update -qq
                                            apt-get install -y ${env.DEBIAN_DEPENDENCIES} || exit 1
                                        else
                                            echo "无法识别的Linux发行版"
                                            exit 1
                                        fi
                                    }
                                    
                                    # 安装依赖（参数：RHEL依赖包 Debian依赖包）
                                    install_deps
                                    
                                    # 2. 验证环境
                                    echo "===== 环境验证 ====="
                                    g++ --version || { echo "[ERROR] g++未安装"; exit 1; }
                                    cmake --version || cmake3 --version || { echo "[ERROR] cmake未安装"; exit 1; }

                                    # 3. 创建软连接（注意路径调整）
                                    echo "===== 创建软连接 ====="
                                    cd lib64
                                    ln -sf libaws-c-auth.so.1.0.0 libaws-c-auth.so
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

                                    echo "===== 当前动态库 ====="
                                    ls -al lib64

                                    # 4. 编译并指定输出目录
                                    echo "===== 开始编译 ====="
                                    mkdir -p ${env.BUILD_OUT_DIR}
                                    mkdir -p ${env.BUILD_ERR_DIR}

                                    make clean
                                    
                                    # 编译到buildout目录
                                    make -j\$(nproc) 2> ${env.BUILD_ERR_DIR}/build_errors.log || {
                                        cat ${env.BUILD_ERR_DIR}/build_errors.log
                                        exit 1
                                    }
                                    
                                    # 运行测试
                                    if grep -q "^test:" Makefile; then
                                        make test 2> ${env.BUILD_ERR_DIR}/test_errors.log || {
                                            cat ${env.BUILD_ERR_DIR}/test_errors.log
                                            exit 1
                                        }
                                    fi
                                '
                            """,
                            returnStatus: true
                        )
                        
                        // 检查编译结果（原有逻辑保持不变）
                        if (compileResult != 0) {
                            error "❌ 容器内编译测试失败，返回码: ${compileResult}"
                        }

                        echo "✅ 容器内编译测试完成"
                    } catch (Exception e) {
                        // 捕获异常时调整日志路径到buildout目录
                        def errorLog = sh(
                            script: "cat ${WORKSPACE}/${env.BUILD_ERR_DIR}/build_errors.log ${WORKSPACE}/${env.BUILD_ERR_DIR}/test_errors.log 2>/dev/null || true",
                            returnStdout: true
                        )
                        error "❌ 编译测试阶段失败:\n${e.getMessage()}\n错误日志:\n${errorLog}"
                    }
                }
            }
        }

        stage('构建应用镜像') {
            steps {
                echo "当前处理节点: ${env.NODE_NAME}"
                script {
                    try {
                        def tag = env.BUILD_NUMBER
                        env.IMAGE_TAG = tag
                        echo "构建应用镜像: ${APP_IMAGE_URL}:${tag}"
                        
                        dir("${WORKSPACE}/dockfile") {
                            // 将编译产物拷贝到dockerfile目录下，使Dockerfile和应用在同一目录dockfile下，根据构建应用镜像要用到的文件做相应修改
                            sh """
                                cp ${WORKSPACE}/${env.BUILD_OUT_DIR}/frssvr ./
                                cp -r ${WORKSPACE}/lib64 ./
                            """
                            
                            // 构建镜像
                            sh "docker build -t ${APP_IMAGE_URL}:${tag} ."
                        }
                        
                        echo "✅ 应用镜像构建成功"
                    } catch (Exception e) {
                        error "❌ 应用镜像构建失败: ${e.getMessage()}"
                    }
                }
            }
        }

        stage('上传应用镜像') {
            steps {
                echo "当前处理节点: ${env.NODE_NAME}"
                script {
                    try {
                        echo "登录Docker仓库..."
                        withCredentials([usernamePassword(credentialsId: 'docker-registry-creds', 
                                        usernameVariable: 'DOCKER_USER', 
                                        passwordVariable: 'DOCKER_PASS')]) {
                            sh "echo ${DOCKER_PASS} | docker login -u ${DOCKER_USER} --password-stdin ${DOCKER_REGISTRY}"
                        }
                        
                        echo "上传应用镜像 ${APP_IMAGE_URL}:${IMAGE_TAG} 到仓库..."
                        sh "docker push ${APP_IMAGE_URL}:${IMAGE_TAG}"
                        
                        // 同时标记为latest并推送
                        sh "docker tag ${APP_IMAGE_URL}:${IMAGE_TAG} ${APP_IMAGE_URL}:latest"
                        sh "docker push ${APP_IMAGE_URL}:latest"
                        
                        echo "✅ 应用镜像上传成功"
                    } catch (Exception e) {
                        error "❌ 应用镜像上传失败: ${e.getMessage()}"
                    } finally {
                        // 登出Docker仓库
                        sh "docker logout ${DOCKER_REGISTRY}"
                    }
                }
            }
        }


/*
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
                        echo "✅ Kubernetes部署完成"
                    } catch (Exception e) {
                        error "❌ Kubernetes部署失败: ${e.getMessage()}"
                    }
                }
            }
        }
*/
    }
    
    post {
        always {
            echo "清理工作空间..."
            sh "rm -rf ${WORKSPACE}/${env.BUILD_ERR_DIR}"
            cleanWs()
        }
        success {
            echo "✅ 流水线执行成功"
            // 可以添加通知逻辑
        }
        failure {
            echo "❌ 流水线执行失败"
            // 可以添加失败通知逻辑
        }
    }
}