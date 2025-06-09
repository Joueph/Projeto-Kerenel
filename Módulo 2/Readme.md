Módulo 2 – Sistema de Pontuação de Comportamento de Processos
📌 Descrição
Este módulo foi desenvolvido com o objetivo de monitorar e classificar o comportamento de processos ativos em sistemas Linux, atribuindo uma nota de risco (Baixo, Médio ou Alto) com base no uso de CPU, chamadas de sistema e operações de E/S (entrada/saída).

O design prioriza simplicidade, eficiência e integração com a estrutura do sistema de arquivos virtual /proc, que é o meio nativo de exposição de informações no kernel Linux.

⚙️ Detalhes Técnicos
🔍 Coleta de Informações Leitura dos arquivos:

/proc/[pid]/stat

/proc/[pid]/io

Uso de bibliotecas do Linux para acesso eficiente.

Cálculo de métricas por processo por meio de contadores internos.

📊 Cálculo de Risco
Classificação em três níveis: Baixo, Médio e Alto.

Avaliação com base em limites empíricos definidos durante o desenvolvimento.

Métricas consideradas:

Uso de CPU

Volume de operações de I/O

Número de chamadas de sistema

🖥️ Interface com o Usuário
Os resultados são expostos por meio de uma entrada personalizada em /proc.

A leitura pode ser feita com o comando:

bash Copiar Editar cat /proc/riskmonitor ✅ Testes Realizados 🧪 Procedimentos de Validação Execução de processos leves e pesados.

Monitoramento do conteúdo de /proc/riskmonitor em diferentes estágios de carga.

📈 Resultados Observados
Processos com alto uso de CPU classificados corretamente como Médio ou Alto risco.

Processos com operações intensas de I/O (ex: cópia de arquivos grandes) corretamente identificados como de risco aumentado.

Estabilidade comprovada: sem travamentos ou vazamentos de memória.

🧩 Desafios e Soluções
⚠️ Leitura Precisa de I/O Desafio: Incompatibilidade de métodos antigos com versões recentes do kernel.

Solução: Leitura direta de /proc/[pid]/io e cálculo da diferença entre read_bytes e write_bytes ao longo do tempo.

🚀 Instruções de Instalação
🔧 Requisitos Sistema Ubuntu Linux com ferramentas de build para módulos do kernel.

Arquivos: Makefile e riskmonitor.c na mesma pasta.

📦 Passo a Passo
Compile o módulo:

bash Copiar Editar make Carregue o módulo no kernel:

bash Copiar Editar sudo insmod riskmonitor.ko Visualize os dados coletados:

bash Copiar Editar cat /proc/riskmonitor Se o comando acima não funcionar, tente:

bash Copiar Editar cd /proc cat riskmonitor Para remover o módulo:

bash Copiar Editar sudo rmmod riskmonitor 📁 Estrutura de Arquivos Copiar Editar . ├── Makefile ├── riskmonitor.c └── README.md 📌 Observações Finais Este módulo é uma ferramenta útil para análise de comportamento de processos no Linux, com potencial para aplicações em ambientes de monitoramento, segurança e ensino de sistemas operacionais.
